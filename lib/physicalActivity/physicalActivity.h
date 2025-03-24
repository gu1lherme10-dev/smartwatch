#ifndef PHYSICAL_ACTIVITY_H
#define PHYSICAL_ACTIVITY_H

#include <vector>
#include <config.h>
#include <Preferences.h>

struct ActivityEvent {
    uint32_t timestamp;
    uint32_t data;    
};

class PhysicalActivity {
public:
    explicit PhysicalActivity(TTGOClass *watch);
    void begin();
    bool checkStep();
    uint32_t getStepCount();
    uint32_t getStepCountMock();
    void updateActivity();
    void storeActivityEvent(uint32_t steps, uint8_t activity);
    static void IRAM_ATTR onInterrupt();

private:
    TTGOClass *watch;
    BMA *sensor;
    uint32_t lastStepCount;
    uint32_t lastUpdateTime;
    uint8_t currentActivity;
    std::vector<ActivityEvent> activityResume;
    static volatile bool irq;
    String getCurrentDateKey();

};

#endif // PHYSICAL_ACTIVITY_H
