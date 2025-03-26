#include "physicalActivity.h"
#include <ctime>
#include "config.h"
#include "utils.h"
#include "nvs.h"
#include "nvs_flash.h"

volatile bool PhysicalActivity::irq = false; // Flag da interrupção

#define STEP_THRESHOLD_WALK 40  
#define STEP_THRESHOLD_RUN 140
Preferences preferences;

// Construtor
PhysicalActivity::PhysicalActivity(TTGOClass *watch) {
    this->watch = watch;
    this->sensor = watch->bma;
    this->lastStepCount = 0;
    this->currentActivity = -1;
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

void PhysicalActivity::updateActivity() {
    uint32_t now = millis();
    if (now - lastUpdateTime < 60000) return;
    uint32_t currentSteps = getStepCount();
    uint32_t stepDelta = currentSteps - lastStepCount;
    uint8_t detectedActivity = detectActivity(stepDelta);
    if (currentActivity == -1) {
        Serial.println("Iniciando monitoramento. Atividade detectada: " + String(detectedActivity));
        currentActivity = detectedActivity;
    } else if (detectedActivity != currentActivity) {
        Serial.println("Mudança de atividade: " + String(currentActivity) + " -> " + String(detectedActivity));
        storeActivityEvent(currentSteps, currentActivity);
        currentActivity = detectedActivity;
    }

    lastStepCount = currentSteps;
    lastUpdateTime = now;
}

uint8_t PhysicalActivity::detectActivity(uint32_t stepDelta) {
    if (stepDelta >= STEP_THRESHOLD_RUN) return 2; // Correndo
    if (stepDelta > 0) return 1; // Caminhando
    return 0;
}


String PhysicalActivity::getCurrentDateKey() {
    RTC_Date date = watch->rtc->getDateTime();

    String dateKey = String(date.year) + (date.month < 10 ? "0" : "") + String(date.month) + (date.day < 10 ? "0" : "") + String(date.day);
    return dateKey;
}

void PhysicalActivity::storeActivityEvent(uint32_t steps, uint8_t activity) {
    Serial.println("========== Salvando Evento ==========");

    RTC_Date date = watch->rtc->getDateTime();
    uint32_t timestamp = convertToUnixTimestamp(date); 
    Serial.print("Timestamp: ");
    Serial.println(timestamp);

    ActivityEvent event;
    event.timestamp = timestamp;
    event.data = (steps << 2) | activity;  

    Serial.print("Passos: ");
    Serial.println(steps);

    Serial.print("Tipo de Atividade: ");
    Serial.println(activity);

    Serial.print("Data (codificado): ");
    Serial.println(event.data, BIN);

    uint8_t buffer[sizeof(ActivityEvent)];
    memcpy(buffer, &event, sizeof(ActivityEvent));

    preferences.begin("activity", false);

    String timestampKey = String(timestamp);
    bool success = preferences.putBytes(timestampKey.c_str(), buffer, sizeof(ActivityEvent));
    
    preferences.end();
}

void PhysicalActivity::printEventsForDay(uint32_t dayTimestamp) {
    preferences.begin("activity", true);

    Serial.println("========== Eventos do Dia ==========");

    nvs_iterator_t it = nvs_entry_find("nvs", "activity", NVS_TYPE_BLOB);
    while (it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);

        uint32_t timestamp = String(info.key).toInt();

        if (timestamp >= dayTimestamp && timestamp < dayTimestamp + 86400) {
            uint8_t buffer[sizeof(ActivityEvent)];
            size_t dataSize = preferences.getBytes(info.key, buffer, sizeof(ActivityEvent));

            if (dataSize == sizeof(ActivityEvent)) {
                ActivityEvent event;
                memcpy(&event, buffer, sizeof(ActivityEvent));

                uint32_t steps = event.data >> 2;
                uint8_t activity = event.data & 0b11;

                Serial.print("Timestamp: ");
                Serial.println(event.timestamp);

                Serial.print("Passos: ");
                Serial.println(steps);

                Serial.print("Atividade: ");
                if (activity == 0) {
                    Serial.println("Parado");
                } else if (activity == 1) {
                    Serial.println("Caminhando");
                } else if (activity == 2) {
                    Serial.println("Correndo");
                }

                Serial.println("-----------------------------");
            }
        }
    }

    preferences.end();
}
