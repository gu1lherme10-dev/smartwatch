#ifndef PHYSICAL_ACTIVITY_SERVICE_H
#define PHYSICAL_ACTIVITY_SERVICE_H

#include "BLEServiceBase.h"
#include "physicalActivity.h"
#include "activityEvent.h"

class PhysicalActivityService : public BLEServiceBase {
public:
    PhysicalActivityService(PhysicalActivity* physicalActivity);
        
    void begin() override;
    void loop() override;
    
    static PhysicalActivityService* instance;
    static bool appIsActive;
    
    void notifyBufferOverflow();
    std::vector<uint8_t> serializeEvents(const std::vector<ActivityEvent>& events);
    void sendActivityEventsBLE(BLEDevice central, BLECharacteristic characteristic);
    static void sendActivitySummary(BLEDevice central, BLECharacteristic characteristic);
    void notifyNewActivityEvent(ActivityEvent event);
    void updateInactiveStatus(BLEDevice central, BLECharacteristic characteristic);
    static void updateInactiveStatusStatic(BLEDevice central, BLECharacteristic characteristic);

private:
    PhysicalActivity *physicalActivity;
    BLEService physicalActivityService;
    BLECharacteristic readActivitySummary;
    BLEUnsignedCharCharacteristic bufferOverflowNotify;
    BLECharacteristic notifyNewActivity;
    BLEUnsignedIntCharacteristic inactiveStatus;
};

#endif
