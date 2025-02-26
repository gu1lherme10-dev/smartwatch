#include "battery.h"
#include <Wire.h>
#include <config.h>
#include <utils.h>

// Private variables
AXP20X_Class power; 
unsigned long startMillisBattery;
unsigned long currentMillisBattery;
const unsigned long period = 2000;  
float batteryAverageVoltage = 0.00;

void setupBatteryMonitor()
{
    Wire.begin();
    power.begin(); 

    if (!power.begin()) 
    {
        Serial.println("Falha ao inicializar o AXP202!");
        while (1); 
    }

    power.setPowerOutPut(AXP202_LDO2, AXP202_ON); 
    power.setLDO2Voltage(3300);
    power.setChgLEDMode(AXP20X_LED_LOW_LEVEL);
    

   
    initializeMovingAverage(10);  
    startMillisBattery = millis();  
}

void calculateBatteryVoltage()
{
    
    float batteryVoltage = power.getBattVoltage();
    batteryAverageVoltage = movingMediaFilter(batteryVoltage);  
}

int getBatteryPercentage()
{
    if (isChargingBattery())  
    {
        float batRef = 4.2;  
        return map(batteryAverageVoltage, 3.0, batRef, 0, 100);  
    }
    else
    {
        return -1; 
    }
}

int getBatteryStatus()
{
    if (!isChargingBattery()) 
    {
        return 0;  // Bateria não conectada
    }
    else if (getBatteryPercentage() >= 0 && getBatteryPercentage() < 20)
    {
        return 1;  // Bateria muito baixa
    }
    else if (getBatteryPercentage() >= 20 && getBatteryPercentage() < 50)
    {
        return 2;  // Bateria média
    }
    else if (getBatteryPercentage() >= 50 && getBatteryPercentage() < 95)
    {
        return 3;  // Bateria boa
    }
    else if (getBatteryPercentage() > 95)
    {
        return 4;
    }
    return -1;  // Caso de erro
}

bool isChargingBattery()
{
    return power.isChargeing();
}

void loopMonitoringBattery()
{
    currentMillisBattery = millis();
    if (currentMillisBattery - startMillisBattery >= period)
    {
        calculateBatteryVoltage(); 
        startMillisBattery = currentMillisBattery; 
    }
}

bool isBateryLowLevel()
{
    return getBatteryPercentage() < 20;
}
