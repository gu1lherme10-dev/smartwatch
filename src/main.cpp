#include "bluetooth.h"
#include "battery.h"
#include "battery/BatteryService.h"
#include "physicalActivity.h"
#include "screenManager.h"
#include "physicalActivity/physicalActivityService.h"
#include "fallDetection/fallDetectionService.h"

Bluetooth bluetooth;

unsigned long lastBatteryCheck = 0;
const unsigned long batteryCheckInterval = 10000;

TTGOClass *watch;
BatteryMonitor *batteryMonitor;
PhysicalActivity *physicalActivity;
BatteryBLEService *batteryService;
TFT_eSPI *tft;
ScreenManager *screenManager;
PhysicalActivityService *physicalActivityService;
FallDetectionService *fallDetectionService;

void setup()
{
    Serial.begin(115200);

    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();
    tft = watch->tft;

    batteryMonitor = new BatteryMonitor(watch);
    physicalActivity = new PhysicalActivity(watch);
    batteryService = new BatteryBLEService(batteryMonitor);
    fallDetectionService = new FallDetectionService();
    physicalActivityService = new PhysicalActivityService(physicalActivity);
    screenManager = new ScreenManager(watch);

    physicalActivity->setCallback(physicalActivityService->notifyNewActivityCallback());

    batteryMonitor->begin();
    bluetooth.begin();
    physicalActivity->begin();
    physicalActivityService->begin();
    batteryService->begin();
    fallDetectionService->begin();

    physicalActivity->getEventsForDay();
}

void loop()
{
    screenManager->isPressed() || !screenManager->screenTimeout() ? screenManager->turnOn() : screenManager->turnOff();
    bluetooth.loop();

    physicalActivity->updateActivity();
    screenManager->showStepCount(physicalActivity->getStepCount());

    unsigned long currentMillis = millis();
    if (currentMillis - lastBatteryCheck >= batteryCheckInterval)
    {
        lastBatteryCheck = currentMillis;

        if (batteryMonitor->isBatteryLowLevel())
        {
            Serial.println("Battery low level");
            batteryService->notifyBatteryLowLevel();
        }

        if (screenManager->isOn())
        {
            // screenManager->displayActivitySummary(physicalActivity->getActivitySummary());
            screenManager->updateBattery(batteryMonitor->getBatteryPercentage());
        }
    }

    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "show")
        {
            Serial.println("Exibindo eventos salvos...");
            physicalActivity->getEventsForDay();
        }
        else if (command == "delete")
        {
            Serial.println("Apagando todos os eventos...");
            physicalActivity->deleteAllEvents();
        }
        else if (command == "delete_steps")
        {
            Serial.println("Apagando todos os steps...");
            physicalActivity->resetStepCounter();
        }
        else if (command == "fall")
        {
            Serial.println("Notificando Queda");
            fallDetectionService->notifyFall();
        }
        else
        {
            Serial.println("Comando inválido! Use:");
            Serial.println(" - show → Mostra os eventos salvos");
            Serial.println(" - delete → Apaga todos os eventos");
            Serial.println(" - delete_steps → Apaga todos os passos");
        }
    }
}
