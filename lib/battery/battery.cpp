#include "battery.h"
#include <Arduino.h>
#include <utils.h>
#include <bluetooth.h>

// Definições de pinos para ESP32
#define PIN_BATTERY_STATUS  17  // Ajuste conforme o hardware
#define PIN_BATTERY_VOLTAGE 32
#define PIN_CHARGE_CURRENT  13
#define PIN_ENABLE_BATTERY  14
#define PIN_VBAT            35  // Pino ADC para leitura da bateria

// Variáveis privadas
unsigned long startMillisBattery;
unsigned long currentMillisBattery;
const unsigned long period = 2000;
volatile float batteryAverageVoltage = 0.00;

// Construtor
BatteryMonitor::BatteryMonitor() {}

// Inicia o monitoramento da bateria
void BatteryMonitor::begin() {
    pinMode(PIN_BATTERY_STATUS, INPUT);
    pinMode(PIN_BATTERY_VOLTAGE, INPUT);
    pinMode(PIN_CHARGE_CURRENT, OUTPUT);
    pinMode(PIN_ENABLE_BATTERY, OUTPUT);
    digitalWrite(PIN_CHARGE_CURRENT, LOW);
    digitalWrite(PIN_ENABLE_BATTERY, LOW);

    analogReadResolution(12);

    initializeMovingAverage(10);   // Inicializa o filtro de média móvel
    startMillisBattery = millis(); // Marca o tempo inicial

    Serial.println("Monitoramento de bateria iniciado!");
}

// Calcula a voltagem da bateria
void BatteryMonitor::calculateBatteryVoltage() {
    static float simulatedVoltage = 0.4;

    float newSample = simulatedVoltage;
    batteryAverageVoltage = movingMediaFilter(newSample);

    Serial.print("Tensão simulada: ");
    Serial.print(simulatedVoltage);
    Serial.println(" V");

    Serial.print("Tensão média: ");
    Serial.print(batteryAverageVoltage);
    Serial.println(" V");
}

// Retorna a porcentagem da bateria
int BatteryMonitor::getBatteryPercentage() {
    if (!isChargingBattery()) {
        return -1; // Bateria não conectada
    }

    float minVoltage = 0.0 * 1000;
    float maxVoltage = 5.0 * 1000; 
    int percentage = map(batteryAverageVoltage * 1000, minVoltage, maxVoltage, 0, 100);
    Serial.println(percentage);
    percentage = constrain(percentage, 0, 100);

    return percentage;
}

// Retorna o status da bateria
int BatteryMonitor::getBatteryStatus() {
    if (!isChargingBattery()) {
        return 0; // Bateria não conectada
    } else if (getBatteryPercentage() >= 0 && getBatteryPercentage() < 20) {
        return 1; // Bateria muito baixa
    } else if (getBatteryPercentage() >= 20 && getBatteryPercentage() < 50) {
        return 2; // Bateria média
    } else if (getBatteryPercentage() >= 50 && getBatteryPercentage() < 95) {
        return 3; // Bateria boa
    } else if (getBatteryPercentage() > 95) {
        return 4; // Bateria cheia
    }
    return -1; // Erro
}

// Verifica se a bateria está carregando
bool BatteryMonitor::isChargingBattery() {
    return digitalRead(PIN_BATTERY_STATUS) == LOW;
}

// Função de loop para monitorar a bateria
void BatteryMonitor::loop() {
    currentMillisBattery = millis();
    if (currentMillisBattery - startMillisBattery >= period) {
        calculateBatteryVoltage();
        startMillisBattery = currentMillisBattery;
    }
}

// Verifica se a bateria está com nível baixo
bool BatteryMonitor::isBatteryLowLevel() {
    return (getBatteryPercentage() >= 0 && getBatteryPercentage() < 20);
}
