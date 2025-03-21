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
    BatteryMonitor *batteryMonitor;

    // Ponteiro estático para a instância da classe
    static Bluetooth* instance;

    // Métodos estáticos para manipulação de eventos
    static void updateBatteryLevelCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic);
    static void updateBatteryStatusCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic);

public:
    Bluetooth(BatteryMonitor *batteryMonitor);
    
    void begin();
    void notifyBatteryLowLevel();
    void loop();

    // Métodos não estáticos
    void updateBatteryLevelCharacteristic(BLEDevice device, BLECharacteristic characteristic);
    void updateBatteryStatusCharacteristic(BLEDevice device, BLECharacteristic characteristic);
    
    // Funções estáticas para manipulação de eventos
    static void onBLEConnected(BLEDevice central);
    static void onBLEDisconnected(BLEDevice central);
};

#endif
