#ifndef BATTERY_H
#define BATTERY_H

#include <Wire.h>
#include <utils.h>
#include <config.h>
// Classe para monitoramento de bateria
class BatteryMonitor {
private:
    TTGOClass *watch;
    AXP20X_Class *power; // Instância do módulo de alimentação
    unsigned long startMillisBattery;  // Tempo de início do monitoramento
    unsigned long currentMillisBattery;  // Tempo atual
    const unsigned long period = 2000;  // Período de monitoramento (2 segundos)
    float batteryAverageVoltage;  // Média móvel da voltagem da bateria

public:
    // Construtor
    BatteryMonitor(TTGOClass *watch);

    void begin();

    int getBatteryPercentage();

    int getBatteryStatus();

    bool isBatteryCharging();

    bool isBatteryConnect();

    void loop();

    bool isBatteryLowLevel();
};

#endif // BATTERY_H