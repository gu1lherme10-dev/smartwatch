#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <ArduinoBLE.h>
#include "battery.h"

class Bluetooth {
private:
    BLEService batteryService;
    BLEIntCharacteristic readBatteryLevel;
    BLEIntCharacteristic readBatteryStatus;
    BLEByteCharacteristic notifyLowLevelBattery;
    bool isCentralConnected;
    BatteryMonitor *batteryMonitor; // Referência para o monitor da bateria

public:
    Bluetooth(BatteryMonitor *batteryMonitor);
    
    void begin();
    void updateBatteryLevel();
    void updateBatteryStatus();
    void notifyBatteryLowLevel();
    void loop();
};

#endif
