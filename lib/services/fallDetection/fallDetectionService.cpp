#include "FallDetectionService.h"

#define FALL_SERVICE_UUID "0x1802"
#define FALL_NOTIFY_UUID "0x2A46"

FallDetectionService* FallDetectionService::instance = nullptr;

FallDetectionService::FallDetectionService()
    : fallDetectionService(FALL_SERVICE_UUID),
      notifyFallDetection(FALL_NOTIFY_UUID, BLENotify)
{
    instance = this;
    lastNotificationTime = 0;
}

void FallDetectionService::begin()
{
    fallDetectionService.addCharacteristic(notifyFallDetection);
    BLE.addService(fallDetectionService);

    notifyFallDetection.setValue(0);

}

void FallDetectionService::loop()
{
    unsigned long currentTime = millis();
    if (currentTime - lastNotificationTime >= notificationInterval)
    {
        notifyFall();
        lastNotificationTime = currentTime;
    }
}

void FallDetectionService::notifyFall() {
    Serial.println("Notificando Queda");
    notifyFallDetection.writeValue(1);
}

