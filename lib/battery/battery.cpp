#include "battery.h"
#include <Wire.h>
#include <config.h>
#include <utils.h>

// Variáveis privadas
volatile AXP20X_Class *power;
unsigned long startMillisBattery;
unsigned long currentMillisBattery;
const unsigned long period = 2000;
volatile float batteryAverageVoltage = 0.00;

// Construtor
BatteryMonitor::BatteryMonitor(TTGOClass *watch) {
    this->watch = watch;
    this->power = watch->power;
}

// Inicia o monitoramento da bateria
void BatteryMonitor::begin() {
    Wire.begin();

    if (!power) {
        Serial.println("Falha ao inicializar o AXP202!");
        while (1);  // Trava o sistema caso haja falha
    }

    power->setPowerOutPut(AXP202_LDO2, AXP202_ON);
    power->setLDO2Voltage(3300);
    power->setChgLEDMode(AXP20X_LED_LOW_LEVEL);

    initializeMovingAverage(10);  // Inicializa o filtro de média móvel
    startMillisBattery = millis();  // Marca o tempo de início

    Serial.println("Monitoramento de bateria iniciado!");
}

// Calcula a voltagem da bateria
void BatteryMonitor::calculateBatteryVoltage() {
    float batteryVoltage = power->getBattVoltage();
    batteryAverageVoltage = movingMediaFilter(batteryVoltage);  // Atualiza a média móvel
}

// Retorna a porcentagem da bateria
int BatteryMonitor::getBatteryPercentage() {
    if (isChargingBattery()) {
        float batRef = 4.2;  // Valor de referência para a carga total da bateria
        return map(batteryAverageVoltage, 3.0, batRef, 0, 100);
    }
    return -1;  // Se não estiver carregando, retorna erro
}

// Retorna o status da bateria
int BatteryMonitor::getBatteryStatus() {
    if (!isChargingBattery()) {
        return 0;  // Bateria não conectada
    } else if (getBatteryPercentage() >= 0 && getBatteryPercentage() < 20) {
        return 1;  // Bateria muito baixa
    } else if (getBatteryPercentage() >= 20 && getBatteryPercentage() < 50) {
        return 2;  // Bateria média
    } else if (getBatteryPercentage() >= 50 && getBatteryPercentage() < 95) {
        return 3;  // Bateria boa
    } else if (getBatteryPercentage() > 95) {
        return 4;  // Bateria cheia
    }
    return -1;  // Caso de erro
}

// Verifica se a bateria está carregando
bool BatteryMonitor::isChargingBattery() {
    return power->isChargeing();
}

// Função de loop para monitorar a bateria
void BatteryMonitor::loop() {
    currentMillisBattery = millis();
    if (currentMillisBattery - startMillisBattery >= period) {
        calculateBatteryVoltage();  // Atualiza a voltagem da bateria
        startMillisBattery = currentMillisBattery;  // Reseta o contador
    }
}

// Verifica se a bateria está com nível baixo
bool BatteryMonitor::isBatteryLowLevel() {
    return getBatteryPercentage() < 20;
}
