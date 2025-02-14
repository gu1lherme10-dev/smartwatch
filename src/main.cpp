#include "config.h"
#include "StepCounter.h"

TTGOClass *watch;
StepCounter *stepCounter;

void setup() {
    Serial.begin(115200);
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();

    // Inicializa a biblioteca do contador de passos
    stepCounter = new StepCounter(watch);
    stepCounter->begin();
}

void loop() {
    if (stepCounter->checkStep()) {
        uint32_t stepCount = stepCounter->getStepCount();
        Serial.printf("Passos: %d\n", stepCount);
    }
    delay(20);
}
