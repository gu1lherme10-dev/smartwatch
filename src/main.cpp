#include "bluetooth.h"
#include "battery.h"
#include "battery/BatteryService.h"
#include "physicalActivity.h"
#include "screenManager.h"
#include "physicalActivity/physicalActivityService.h"

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

void setup() {
    Serial.begin(115200);

    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();
    tft = watch->tft;

    batteryMonitor = new BatteryMonitor(watch);
    physicalActivity = new PhysicalActivity(watch);
    batteryService = new BatteryBLEService(batteryMonitor);
    physicalActivityService = new PhysicalActivityService();
    screenManager = new ScreenManager(watch);

    batteryMonitor->begin();
    bluetooth.begin();
    physicalActivity->begin();
    physicalActivityService->begin();
    batteryService->begin();
}

void loop() {
    screenManager ->isPressed() || !screenManager->screenTimeout() ? screenManager->turnOn() : screenManager->turnOff();
    bluetooth.loop();

    physicalActivity->updateActivity();
    screenManager->showStepCount(physicalActivity->getStepCount());

    unsigned long currentMillis = millis();
    if (currentMillis - lastBatteryCheck >= batteryCheckInterval) {
        lastBatteryCheck = currentMillis;

        if (batteryMonitor->isBatteryLowLevel()) {
            Serial.println("Battery low level");
            //batteryService->notifyBatteryLowLevel();
        }

        if (screenManager->isOn()) {
            // screenManager->displayActivitySummary(physicalActivity->getActivitySummary());
            screenManager->updateBattery(batteryMonitor->getBatteryPercentage());
        }
    }
}
