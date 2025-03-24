#include "physicalActivity.h"
#include <ctime>

volatile bool PhysicalActivity::irq = false;

#define STEP_THRESHOLD_WALK 40  // Passos/min para caminhar
#define STEP_THRESHOLD_RUN 140  // Passos/min para correr
Preferences preferences;

// Construtor
PhysicalActivity::PhysicalActivity(TTGOClass *watch) {
    this->watch = watch;
    this->sensor = watch->bma;
}

void PhysicalActivity::begin() {
    sensor->begin();

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
    irq = true;
}

bool PhysicalActivity::checkStep() {
    if (irq) {
        irq = false;
        while (!sensor->readInterrupt());
        return sensor->isStepCounter();
    }
    return false;
}

uint32_t PhysicalActivity::getStepCount() {
    return sensor->getCounter();
}

uint32_t PhysicalActivity::getStepCountMock() {
    static uint32_t startTime = millis(); // Marca o tempo inicial
    uint32_t elapsedTime = (millis() - startTime) / 60000; // Tempo decorrido em minutos

    if (elapsedTime < 5) {
        return random(140, 201);
    } else {
        // Após 5 minutos, retorna valores entre 0 e 130
        return random(0, 131);
    }
}

// Verifica passos e atualiza estado da atividade
void PhysicalActivity::updateActivity() {
    if (irq) {
        irq = false;
        while (!sensor->readInterrupt());
        
        uint32_t currentSteps = getStepCountMock();
        Serial.println("Passos: " + String(currentSteps));
        uint32_t now = millis();
        uint32_t elapsedTime = (now - lastUpdateTime) / 60000; // Convertendo para minutos
        
        if (elapsedTime > 0) {
            uint32_t stepRate = (currentSteps - lastStepCount) / elapsedTime;
            uint8_t newActivity = 0;
            
            if (stepRate >= STEP_THRESHOLD_RUN) {
                newActivity = 2;
            } else if (stepRate > 0) {
                newActivity = 1;
            }

            if (newActivity != currentActivity) {
                currentActivity = newActivity;
                storeActivityEvent(currentSteps, currentActivity);
            }

            lastStepCount = currentSteps;
            lastUpdateTime = now;
        }
    }
}

String PhysicalActivity::getCurrentDateKey() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    
    char dateStr[9];
    strftime(dateStr, sizeof(dateStr), "%Y%m%d", timeinfo);
    
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
