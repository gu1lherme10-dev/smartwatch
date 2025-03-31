#ifndef PHYSICAL_ACTIVITY_SERVICE_H
#define PHYSICAL_ACTIVITY_SERVICE_H

#include "BLEServiceBase.h"
#include "physicalActivity.h"

class PhysicalActivityService : public BLEServiceBase {
public:
    PhysicalActivityService();
    
    struct ActivityEvent {
        uint32_t timestamp;
        uint32_t activity_steps;
    };
    
    void begin() override;
    void loop() override;
    
    static PhysicalActivityService* instance;
    static bool appIsActive;
    
    void notifyBufferOverflow();
    std::vector<ActivityEvent> generateActivityEvents(uint32_t startTimestamp, uint32_t numEvents);
    std::vector<uint8_t> serializeEvents(const std::vector<ActivityEvent>& events);
    void sendActivityEventsBLE(BLEDevice central, BLECharacteristic characteristic);
    static void sendActivitySummary(BLEDevice central, BLECharacteristic characteristic);
    void notifyNewActivityEvent(ActivityEvent event);
    void updateInactiveStatus(BLEDevice central, BLECharacteristic characteristic);
    static void updateInactiveStatusStatic(BLEDevice central, BLECharacteristic characteristic);

private:
    BLEService physicalActivityService;
    BLECharacteristic readActivitySummary;
    BLEUnsignedCharCharacteristic bufferOverflowNotify;
    BLECharacteristic notifyNewActivity;
    BLEUnsignedIntCharacteristic inactiveStatus;
};

#endif
