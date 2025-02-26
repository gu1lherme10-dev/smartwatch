#include "config.h"
#include "StepCounter.h"
#include "bluetooth.h"
#include "battery.h"

TTGOClass *watch;
StepCounter *stepCounter;

void setup() {
    Serial.begin(115200);
    setupBluetooth();
    //setupBatteryMonitor();
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();

    // Inicializa a biblioteca do contador de passos
    // stepCounter = new StepCounter(watch);
    // stepCounter->begin();
}

void loop() {
	//loopMonitoringBattery();
	loopPollingBluetooth();

    // if (stepCounter->checkStep()) {
    //     uint32_t stepCount = stepCounter->getStepCount();
    //     Serial.printf("Passos: %d\n", stepCount);
    // }

    delay(20);
}
