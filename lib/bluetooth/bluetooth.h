// Bluetooth.h
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <ArduinoBLE.h>
#include <vector>
#include "BLEServiceBase.h"

class Bluetooth {
public:
    static Bluetooth* instance;

    Bluetooth();
    void begin();
    void loop();
    void addService(BLEServiceBase* service);

    static void onBLEConnected(BLEDevice central);
    static void onBLEDisconnected(BLEDevice central);

private:
    std::vector<BLEServiceBase*> services;
};

#endif
