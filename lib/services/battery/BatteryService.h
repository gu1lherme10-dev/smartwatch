#ifndef BATTERYSERVICE_H
#define BATTERYSERVICE_H

#include "BLEServiceBase.h"
#include "battery.h"

class BatteryBLEService : public BLEServiceBase {
public:
    BatteryBLEService(BatteryMonitor* batteryMonitor);
    void begin() override;
    void loop() override;
    void notifyBatteryLowLevel();
    static BatteryBLEService* instance;
    static void updateBatteryLevelCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic);
    static void updateBatteryStatusCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic);
private:
    BatteryMonitor* batteryMonitor;
    BLEService batteryService;
    BLEUnsignedCharCharacteristic readBatteryLevel;
    BLEUnsignedCharCharacteristic readBatteryStatus;
    BLEUnsignedCharCharacteristic notifyLowLevelBattery;

    void updateBatteryLevelCharacteristic(BLEDevice central, BLECharacteristic characteristic);
    void updateBatteryStatusCharacteristic(BLEDevice central, BLECharacteristic characteristic);
};

#endif
