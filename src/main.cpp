#include "bluetooth.h"
#include "battery.h"
#include "battery/BatteryService.h"
#include "physicalActivity.h"

Bluetooth bluetooth;

unsigned long lastBatteryCheck = 0;
const unsigned long batteryCheckInterval = 10000;
TTGOClass *watch;
BatteryMonitor batteryMonitor(watch);
PhysicalActivity physicalActivity(watch);
BatteryBLEService batteryService(&batteryMonitor);

void setup() {
    Serial.begin(115200);

    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();

    batteryMonitor.begin();
    bluetooth.begin();
    physicalActivity.begin();
    bluetooth.addService(&batteryService);
}

void loop() {
    batteryMonitor.loop();
    bluetooth.loop();

    //physicalActivity.updateActivity();

    unsigned long currentMillis = millis();
    if (currentMillis - lastBatteryCheck >= batteryCheckInterval) {
        lastBatteryCheck = currentMillis;

        if (batteryMonitor.isBatteryLowLevel()) {
            Serial.println("Battery low level");
            batteryService.notifyBatteryLowLevel();
        }
    }
}
