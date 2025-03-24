#include "bluetooth.h"
#include "battery.h"
#include "battery/BatteryService.h"
#include "physicalActivity.h"

BatteryMonitor batteryMonitor;
PhysicalActivity physicalActivity(&watch);

BatteryBLEService batteryService(&batteryMonitor);

Bluetooth bluetooth;

unsigned long lastBatteryCheck = 0;
const unsigned long batteryCheckInterval = 10000;

void setup() {
    Serial.begin(115200);

    batteryMonitor.begin();
    bluetooth.begin();
    physicalActivity.begin();
    bluetooth.addService(&batteryService);
}

void loop() {
    batteryMonitor.loop();
    bluetooth.loop();

    physicalActivity.updateActivity();

    unsigned long currentMillis = millis();
    if (currentMillis - lastBatteryCheck >= batteryCheckInterval) {
        lastBatteryCheck = currentMillis;

        if (batteryMonitor.isBatteryLowLevel()) {
            Serial.println("Battery low level");
            batteryService.notifyBatteryLowLevel();
        }
    }
}
