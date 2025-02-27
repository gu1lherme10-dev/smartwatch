#include <ArduinoBLE.h>
#include <battery.h>
#include "bluetooth.h"

Bluetooth::Bluetooth(BatteryMonitor *batteryMonitor)
    : batteryService("0x180F"),
      readBatteryLevel("0x2A19", BLERead),  
      readBatteryStatus("0x2BED", BLERead),
      notifyLowLevelBattery("0x2BE9", BLENotify), 
      isCentralConnected(false), 
      batteryMonitor(batteryMonitor) {
}

// Inicializa o Bluetooth
void Bluetooth::begin()
{
    Serial.println("Inicializando Bluetooth...");
    if (!BLE.begin())
    {
        Serial.println("Falha ao inicializar o módulo Bluetooth!");
        return;
    }

    BLE.setLocalName("Smartwatch");
    BLE.setAdvertisedService(batteryService);

    batteryService.addCharacteristic(readBatteryLevel);
    batteryService.addCharacteristic(readBatteryStatus);
    BLE.addService(batteryService);

    BLE.setEventHandler(BLEConnected, [](BLEDevice central)
                        {
            Serial.print("Conectado, central: ");
            Serial.println(central.address()); });

    BLE.setEventHandler(BLEDisconnected, [](BLEDevice central)
                        {
            Serial.print("Desconectado, central: ");
            Serial.println(central.address());
            BLE.advertise(); });

    readBatteryLevel.setValue(-1);
    readBatteryStatus.setValue(-1);
    notifyLowLevelBattery.setValue(0);

    BLE.advertise();
    Serial.println("Configuração BLE concluída.");
}

// Atualiza o nível de bateria
void Bluetooth::updateBatteryLevel()
{
    int batteryLevel = batteryMonitor->getBatteryPercentage();
    Serial.print("Atualizando nível de bateria: ");
    Serial.println(batteryLevel);
    readBatteryLevel.writeValue(batteryLevel);
}

// Atualiza o status da bateria
void Bluetooth::updateBatteryStatus()
{
    int batteryStatus = batteryMonitor->getBatteryStatus();
    Serial.print("Atualizando status da bateria: ");
    Serial.println(batteryStatus);
    readBatteryStatus.writeValue(batteryStatus);
}

// Notifica nível baixo de bateria
void Bluetooth::notifyBatteryLowLevel()
{
    int batteryPercentage = batteryMonitor->getBatteryPercentage();
    Serial.print("Verificando nível baixo de bateria: ");
    Serial.println(batteryPercentage);
    if (batteryPercentage < 20)
    {
        Serial.println("Notificando nível baixo de bateria.");
        notifyLowLevelBattery.writeValue(1);
    }
}

// Polling do BLE
void Bluetooth::loop()
{
    BLE.poll();
}
