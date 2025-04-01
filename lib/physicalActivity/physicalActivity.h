#ifndef PHYSICAL_ACTIVITY_H
#define PHYSICAL_ACTIVITY_H

#include <vector>
#include <config.h>
#include <Preferences.h>
#include "activityEvent.h"

class PhysicalActivity {
public:
    explicit PhysicalActivity(TTGOClass *watch);
    void begin();
    uint32_t getStepCount();
    void updateActivity();
    uint32_t startTime;
    void storeActivityEvent(uint32_t steps, uint8_t activity);
    static void IRAM_ATTR onInterrupt();
    struct ActivitySummary {
        float idleTime;
        float walkingTime;
        float runningTime;
    };
    String getCurrentDateKey();
    std::vector<ActivityEvent> getEventsForDay();
    uint8_t detectActivity(uint32_t stepDelta);
    uint32_t getDayTimestamp(RTC_Date date);
    void deleteAllEvents();
    void resetStepCounter();

private:
    TTGOClass *watch;
    BMA *sensor;
    uint32_t lastStepCount;
    uint32_t lastUpdateTime;
    int8_t currentActivity;
    std::vector<ActivityEvent> activityResume;
    static volatile bool irq;
    uint32_t sessionSteps;
};

#endif // PHYSICAL_ACTIVITY_H
