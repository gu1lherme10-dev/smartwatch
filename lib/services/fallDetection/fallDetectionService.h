#ifndef FALL_DETECTION_SERVICE_H
#define FALL_DETECTION_SERVICE_H

#include "BLEServiceBase.h"
#include <Arduino.h>

class FallDetectionService : public BLEServiceBase
{
public:
    FallDetectionService();
    void begin() override;
    void loop() override;
    static FallDetectionService* instance;
    void notifyFall();

private:
    BLEService fallDetectionService;
    BLEUnsignedCharCharacteristic notifyFallDetection;
    unsigned long lastNotificationTime;
    const unsigned long notificationInterval = 5000; // Notifica a cada 5 segundos
};

#endif
