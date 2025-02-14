#include "stepCounter.h"

volatile bool StepCounter::irq = false;

// Construtor
StepCounter::StepCounter(TTGOClass *watch) {
    this->watch = watch;
    this->sensor = watch->bma;
}

// Inicializa o sensor e configura o contador de passos
void StepCounter::begin() {
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
void IRAM_ATTR StepCounter::onInterrupt() {
    irq = true;
}

// Verifica se um novo passo foi detectado
bool StepCounter::checkStep() {
    if (irq) {
        irq = false;
        while (!sensor->readInterrupt());
        return sensor->isStepCounter();
    }
    return false;
}

// Retorna a contagem de passos
uint32_t StepCounter::getStepCount() {
    return sensor->getCounter();
}
