#ifndef STEP_COUNTER_H
#define STEP_COUNTER_H

#include "config.h"

class StepCounter {
public:
    StepCounter(TTGOClass *watch);
    void begin();
    bool checkStep();
    uint32_t getStepCount();

private:
    TTGOClass *watch;
    BMA *sensor;
    volatile static bool irq;
    static void IRAM_ATTR onInterrupt();
};

#endif // STEP_COUNTER_H
