#ifndef BATTERY_H
#define BATTERY_H

void setupBatteryMonitor();
void loopMonitoringBattery();
int getBatteryPercentage();
bool isChargingBattery();
bool isBateryLowLevel();
int getBatteryStatus();

#endif