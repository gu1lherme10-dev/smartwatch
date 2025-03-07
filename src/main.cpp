#include "config.h"
#include "StepCounter.h"
#include "bluetooth.h"
#include "battery.h"

TTGOClass *watch;
StepCounter *stepCounter;
BatteryMonitor *batteryMonitor;
Bluetooth *bluetooth;

void setup() {
    Serial.begin(115200);
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();

    batteryMonitor = new BatteryMonitor(watch);
    bluetooth = new Bluetooth(batteryMonitor);
    stepCounter = new StepCounter(watch);

    batteryMonitor->begin();
    stepCounter->begin();
    bluetooth->begin();
}

void loop() {
	batteryMonitor->loop();
	bluetooth->loop();

    if (stepCounter->checkStep()) {
        uint32_t stepCount = stepCounter->getStepCount();
        Serial.printf("Passos: %d\n", stepCount);
    }

    delay(20);
}
