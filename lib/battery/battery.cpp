#include "battery.h"
#include <Wire.h>
#include <config.h>
#include <utils.h>

// Private variables
AXP20X_Class power;  // Instancia o objeto AXP20X_Class para o AXP202
unsigned long startMillisBattery;
unsigned long currentMillisBattery;
const unsigned long period = 2000;  // Intervalo de 2 segundos
float batteryAverageVoltage = 0.00;

void setupBatteryMonitor()
{
    Wire.begin();
    power.begin(); // Inicializa a comunicação I2C com o AXP202

    if (!power.begin())  // Verifica se o AXP202 foi inicializado corretamente
    {
        Serial.println("Falha ao inicializar o AXP202!");
        while (1);  // Para o código se não conseguir inicializar o AXP202
    }

    power.setPowerOutPut(AXP202_LDO2, AXP202_ON);  // Habilita a saída LDO2
    power.setLDO2Voltage(3300);  // Define a tensão do LDO2 para 3.3V
    power.setChgLEDMode(AXP20X_LED_LOW_LEVEL);  // Habilita o carregamento da bateria
    // power.enableACIN();  // Habilita a entrada de alimentação externa, se necessário

    // Inicializa o filtro de média móvel
    initializeMovingAverage(10);  // Inicializa com 10 amostras para média móvel
    startMillisBattery = millis();  // Marca o tempo inicial
}

void calculateBatteryVoltage()
{
    // Obtém a tensão da bateria usando o AXP202
    float batteryVoltage = power.getBattVoltage();
    batteryAverageVoltage = movingMediaFilter(batteryVoltage);  // Aplica o filtro de média
}

int getBatteryPercentage()
{
    if (isChargingBattery())  // Se a bateria estiver carregando
    {
        float batRef = 4.2;  // Tensão de referência da bateria LiPo (4.2V totalmente carregada)
        return map(batteryAverageVoltage, 3.0, batRef, 0, 100);  // Mapeia a tensão para uma porcentagem de 0-100%
    }
    else
    {
        return -1;  // Bateria não conectada ou não carregando
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
        return 4;  // Bateria quase cheia
    }
    return -1;  // Caso de erro
}

bool isChargingBattery()
{
    return power.isChargeing();  // Função do AXP202 que verifica o status de carregamento
}

void loopMonitoringBattery()
{
    currentMillisBattery = millis();
    if (currentMillisBattery - startMillisBattery >= period)  // Se passaram 2 segundos
    {
        calculateBatteryVoltage();  // Atualiza a tensão da bateria
        startMillisBattery = currentMillisBattery;  // Reinicia o tempo
    }
}

bool isBateryLowLevel()
{
    return getBatteryPercentage() < 20;
}
