// BLEService.h
#ifndef BLESERVICE_H
#define BLESERVICE_H

#include <ArduinoBLE.h>

class BLEServiceBase {
public:
    virtual void begin() = 0;
    virtual void loop() = 0;
};

#endif
