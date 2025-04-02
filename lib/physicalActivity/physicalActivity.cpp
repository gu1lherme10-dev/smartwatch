#include "physicalActivity.h"
#include "physicalActivity/PhysicalActivityService.h"
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

}

void PhysicalActivity::resetStepCounter() {
    sensor->resetStepCounter();
}

void IRAM_ATTR PhysicalActivity::onInterrupt() {
    irq = true;
}

// Método para obter contagem de passos
uint32_t PhysicalActivity::getStepCount() {
    return sensor->getCounter();
}

void PhysicalActivity::updateActivity() {
    uint32_t now = millis();
    if (now - lastUpdateTime < 60000 || currentActivity == -1) return;  // Atualiza apenas a cada 1 minuto

    uint32_t currentSteps = getStepCount();
    uint32_t stepDelta = currentSteps - lastStepCount;
    lastStepCount = currentSteps;

    uint8_t detectedActivity = detectActivity(stepDelta);

    if (currentActivity == -1) {
        Serial.println("Iniciando monitoramento. Atividade detectada: " + String(detectedActivity));
        currentActivity = detectedActivity;
        sessionSteps = stepDelta;
    }
    else if (detectedActivity != currentActivity) {
        ActivityEvent event;
        Serial.println("now: " + String(now));
        event.timestamp = now;
        event.data = (detectedActivity << 30) | (sessionSteps & 0x3FFFFFFF);  // Codifica atividade + passos
        RTC_Date date = watch->rtc->getDateTime();
        uint32_t timestamp = convertToUnixTimestamp(date);

        if (PhysicalActivityService::appIsActive) {
            event.timestamp = timestamp;
            Serial.println("Notificando nova atividade via BLE!");
            notifyNewActivityCallback(event);       
        }
        else
        {
            Serial.println("App inativo. Salvando.");
            storeActivityEvent(sessionSteps, currentActivity, timestamp);
        }


        sessionSteps = stepDelta;
        currentActivity = detectedActivity;
    }
    else {
        sessionSteps += stepDelta;
    }

    lastUpdateTime = now;
}



uint8_t PhysicalActivity::detectActivity(uint32_t stepDelta) {
    if (stepDelta >= STEP_THRESHOLD_RUN) return 2; // Correndo
    if (stepDelta > 0) return 1; // Caminhando
    return 0;
}

uint32_t PhysicalActivity::getDayTimestamp(RTC_Date date) {
    struct tm t;
    t.tm_year = date.year - 1900;
    t.tm_mon = date.month - 1;
    t.tm_mday = date.day;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_isdst = -1;

    return mktime(&t);
}

void PhysicalActivity::storeActivityEvent(uint32_t steps, uint8_t activity, uint32_t timestamp) {
    Serial.println("========== Salvando Evento ==========");
    Serial.print("Timestamp: ");
    Serial.println(timestamp);

    ActivityEvent event;
    event.timestamp = timestamp;
    event.data = (activity << 30) | steps;

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
    Serial.println(success ? "Evento salvo com sucesso!" : "Falha ao salvar evento!");
    preferences.end();
}

std::vector<ActivityEvent> PhysicalActivity::getEventsForDay() {
    std::vector<ActivityEvent> events;

    preferences.begin("activity", true);
    Serial.println("========== Eventos do Dia ==========");

    RTC_Date date = watch->rtc->getDateTime();
    uint32_t dayTimestamp = getDayTimestamp(date);

    nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, "activity", NVS_TYPE_BLOB);
    while (it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        Serial.println(info.key);
        it = nvs_entry_next(it);
    }

    it = nvs_entry_find(NVS_DEFAULT_PART_NAME, "activity", NVS_TYPE_BLOB);
    while (it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);

        uint32_t timestamp = atoi(info.key);

        if (timestamp >= dayTimestamp && timestamp < dayTimestamp + 86400) { 
            uint8_t buffer[sizeof(ActivityEvent)];

            if (!preferences.isKey(info.key)) {
                Serial.println("Chave não encontrada: " + String(info.key));
                continue;
            }

            size_t dataSize = preferences.getBytes(info.key, buffer, sizeof(ActivityEvent));
            if (dataSize == 0) {
                Serial.println("Falha ao ler dados da chave: " + String(info.key));
                continue;
            }

            ActivityEvent event;
            memcpy(&event, buffer, sizeof(ActivityEvent));

            uint8_t activity = event.data >> 30;
            uint32_t steps = event.data & 0x3FFFFFFF;
            
            Serial.println("-----------------------------");
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

            events.push_back(event);
        }
    }

    preferences.end();
    return events; 
}


void PhysicalActivity::deleteAllEvents() {
    Serial.println("========== Deletando Todos os Eventos ==========");

    preferences.begin("activity", false);
    
    nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, "activity", NVS_TYPE_BLOB);
    
    if (it == NULL) {
        Serial.println("Nenhum evento encontrado para deletar!");
        preferences.end();
        return;
    }

    while (it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        Serial.println("Deletando chave: " + String(info.key));
        preferences.remove(info.key);
        it = nvs_entry_next(it);
    }

    preferences.end();
    Serial.println("Todos os eventos foram deletados!");
}

void PhysicalActivity::setCallback(std::function<void(ActivityEvent)> callback) {
    notifyNewActivityCallback = callback;
}
