#include "physicalActivity.h"
#include <ctime>
#include "config.h"

volatile bool PhysicalActivity::irq = false; // Flag da interrupção

#define STEP_THRESHOLD_WALK 40  
#define STEP_THRESHOLD_RUN 140
Preferences preferences;
bool simulationMode = false;  

// Construtor
PhysicalActivity::PhysicalActivity(TTGOClass *watch) {
    this->watch = watch;
    this->sensor = watch->bma;
}

void PhysicalActivity::begin() {
    sensor->begin();

    // Configuração do acelerômetro
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;
    
    sensor->accelConfig(cfg);
    sensor->enableAccel();
    
    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(BMA423_INT1, onInterrupt, RISING);

    sensor->enableFeature(BMA423_STEP_CNTR, true);
    sensor->resetStepCounter();
    sensor->enableStepCountInterrupt();
}

// Handler de interrupção
void IRAM_ATTR PhysicalActivity::onInterrupt() {
    irq = true;  // Marca que um evento de passos ocorreu
}

// Método para obter contagem de passos
uint32_t PhysicalActivity::getStepCount() {
    return sensor->getCounter();
}

uint32_t mockSteps = 0;
uint32_t mockStartTime = 0;
bool isRunning = false;

void IRAM_ATTR PhysicalActivity::mockInterrupt() {
    irq = true;
}


uint32_t PhysicalActivity::getStepCountMock() {
    static uint32_t lastMockTime = 0;
    
    uint32_t now = millis();
    if (now - lastMockTime < 1000) { // Atualiza a cada 1 segundo
        return mockSteps; // Mantém o mesmo valor até o próximo ciclo
    }
    lastMockTime = now;

    uint32_t elapsedMinutes = (millis() - mockStartTime) / 60000;

    if (elapsedMinutes >= 2) { 
        Serial.println("Mock: Alterando estado de atividade...");
        isRunning = !isRunning;  // Alterna entre corrida/caminhada a cada 5 min
        mockStartTime = millis();
    }

    if (isRunning) {
        mockSteps += 10; // Corrida → 10 passos por segundo (~600 por minuto)
    } else {
        mockSteps += 4;  // Caminhada → 4 passos por segundo (~240 por minuto)
    }

    Serial.println("Mock: Total de passos simulados: " + String(mockSteps));
    mockInterrupt();

    return mockSteps;
}


// Verifica passos e atualiza estado da atividade
void PhysicalActivity::updateActivity() {
    if (simulationMode) {
        irq = true; // Ativa interrupção manualmente
    }

    if (!irq) return; // Só atualiza se houve interrupção

    irq = false;
    uint32_t now = millis();
    uint32_t elapsedTime = (now - lastUpdateTime) / 60000; // Tempo em minutos

    uint32_t currentSteps = simulationMode ? getStepCountMock() : getStepCount();
    
    if (elapsedTime > 0) {
        uint32_t stepRate = (currentSteps - lastStepCount) / elapsedTime;
        uint8_t newActivity = 0;


        if (stepRate >= STEP_THRESHOLD_RUN) {
            newActivity = 2; // Correndo
        } else if (stepRate > 0) {
            newActivity = 1; // Caminhando
        }

        // Registra apenas se houve mudança de estado
        if (newActivity != currentActivity) {
            Serial.println("Mudança de atividade detectada!");
            Serial.println("Novo estado: " + String(newActivity));
            Serial.println("Passos acumulados: " + String(currentSteps));

            storeActivityEvent(currentSteps, newActivity);
            currentActivity = newActivity;
            readStoredActivity();
        }

        lastStepCount = currentSteps;
        lastUpdateTime = now;
    }
}

String PhysicalActivity::getCurrentDateKey() {
    if (!watch || !watch->rtc) {
        Serial.println("⚠️ Erro: RTC não está disponível! Usando millis() como fallback.");
        return String(millis() / 1000);
    }

    RTC_Date date = watch->rtc->getDateTime();  // Obtém a data e hora do RTC

    char dateStr[9];
    snprintf(dateStr, sizeof(dateStr), "%04d%02d%02d", date.year, date.month, date.day);

    return String(dateStr);
}

void PhysicalActivity::storeActivityEvent(uint32_t steps, uint8_t activity) {
    String todayKey = getCurrentDateKey();

    ActivityEvent event;
    event.timestamp = millis();
    event.data = (steps << 2) | activity;

    uint8_t buffer[sizeof(ActivityEvent)];
    memcpy(buffer, &event, sizeof(ActivityEvent));

    preferences.begin("activity", false);
    preferences.putBytes(todayKey.c_str(), buffer, sizeof(ActivityEvent));
    preferences.end();

    Serial.println("Evento salvo na Flash para o dia: " + todayKey);
}

void PhysicalActivity::readStoredActivity() {
    String todayKey = getCurrentDateKey();

    ActivityEvent event;
    uint8_t buffer[sizeof(ActivityEvent)];

    preferences.begin("activity", true);
    
    size_t dataSize = preferences.getBytes(todayKey.c_str(), buffer, sizeof(ActivityEvent));

    if (dataSize == sizeof(ActivityEvent)) {
        memcpy(&event, buffer, sizeof(ActivityEvent));

        uint32_t steps = event.data >> 2;
        uint8_t activity = event.data & 0b11;
        uint32_t timestamp = event.timestamp;

        Serial.println("📦 Dados recuperados da Flash:");
        Serial.println("📅 Data: " + todayKey);
        Serial.println("⏰ Timestamp: " + String(timestamp));
        Serial.println("🚶 Passos: " + String(steps));
        Serial.println("🎭 Atividade: " + String(activity)); // 0 = parado, 1 = caminhando, 2 = correndo
    } else {
        Serial.println("⚠️ Nenhum dado encontrado para " + todayKey);
    }

    preferences.end();
}
