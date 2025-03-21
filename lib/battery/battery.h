#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

class BatteryMonitor {
public:
    BatteryMonitor(); // Construtor

    void begin();          // Inicializa o monitoramento da bateria
    void loop();           // Atualiza os valores periodicamente
    int getBatteryPercentage();  // Retorna a porcentagem da bateria
    int getBatteryStatus();      // Retorna o status da bateria
    bool isChargingBattery();    // Verifica se a bateria está carregando
    bool isBatteryLowLevel();    // Verifica se a bateria está com nível baixo

private:
    void calculateBatteryVoltage(); // Calcula a voltagem da bateria
};

#endif // BATTERY_H
