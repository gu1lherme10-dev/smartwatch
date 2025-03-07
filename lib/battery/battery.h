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

    // Inicia o monitoramento da bateria
    void begin();

    // Calcula a voltagem da bateria
    void calculateBatteryVoltage();

    // Retorna a porcentagem de carga da bateria
    int getBatteryPercentage();

    // Retorna o status da bateria (se está baixa, média, boa, etc)
    int getBatteryStatus();

    // Verifica se a bateria está carregando
    bool isChargingBattery();

    // Função para monitorar a bateria no loop
    void loop();

    // Verifica se a bateria está com nível baixo (abaixo de 20%)
    bool isBatteryLowLevel();
};

#endif // BATTERY_H
