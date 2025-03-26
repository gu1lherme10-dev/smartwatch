#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include "physicalActivity.h"

#include <config.h>
#include <TTGO.h>

class ScreenManager {
public:
    ScreenManager(TTGOClass *watch);
    void updateBattery(int batteryPercentage);
    void turnOn();
    void turnOff();
    bool isOn();
    bool isPressed();
    bool screenTimeout();
    void showStepCount(uint32_t steps);
    void displayActivitySummary(PhysicalActivity::ActivitySummary summary);

private:
    TFT_eSPI *tft;
    TTGOClass *watch;
    bool screenOn;
};

#endif
