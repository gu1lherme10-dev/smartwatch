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

BatteryMonitor::BatteryMonitor(TTGOClass *watch) {
    this->watch = watch;
    this->power = watch->power;
}

void BatteryMonitor::begin() {
    Wire.begin();

    if (!power) {
        Serial.println("Falha ao inicializar o AXP202!");
        while (1);
    }

    power->adc1Enable(
        AXP202_VBUS_VOL_ADC1 |
        AXP202_VBUS_CUR_ADC1 |
        AXP202_BATT_CUR_ADC1 |
        AXP202_BATT_VOL_ADC1,
        true);

    startMillisBattery = millis(); 

    Serial.println("Monitoramento de bateria iniciado!");
}


int BatteryMonitor::getBatteryPercentage() {
    Serial.println("Porcentagem da Bateria");
    int batteryPercentage = power->getBattPercentage();
    Serial.println(batteryPercentage);
    return batteryPercentage;
}

int BatteryMonitor::getBatteryStatus() {
    int batteryPercentage = getBatteryPercentage(); 

    if (!isBatteryConnect()) {
        return -1;
    }
    if (batteryPercentage < 20) {
        return 1;
    } else if (batteryPercentage < 50) {
        return 2;
    } else if (batteryPercentage < 95) {
        return 3;
    } else {
        return 4;
    }
}


bool BatteryMonitor::isBatteryConnect() {
    return power->isBatteryConnect();
}

bool BatteryMonitor::isBatteryLowLevel() {
    return getBatteryPercentage() < 20;
}

bool BatteryMonitor::isBatteryCharging() {
    return power->isChargeing();
}