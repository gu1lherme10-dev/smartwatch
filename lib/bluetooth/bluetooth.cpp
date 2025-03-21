#include <ArduinoBLE.h>
#include <battery.h>
#include "bluetooth.h"

Bluetooth* Bluetooth::instance = nullptr;

Bluetooth::Bluetooth(BatteryMonitor *batteryMonitor)
    : batteryService("0x180F"),
      readBatteryLevel("0x2A19", BLERead),  
      readBatteryStatus("0x2BED", BLERead),
      notifyLowLevelBattery("0x2BE9", BLENotify), 
      isCentralConnected(false), 
      batteryMonitor(batteryMonitor) {
    instance = this; 
}

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
    batteryService.addCharacteristic(notifyLowLevelBattery);
    BLE.addService(batteryService);

    BLE.setEventHandler(BLEConnected, onBLEConnected);
    BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
    
    readBatteryLevel.setEventHandler(BLERead, updateBatteryLevelCharacteristicStatic);
    readBatteryStatus.setEventHandler(BLERead, updateBatteryStatusCharacteristicStatic);

    readBatteryLevel.setValue(-1);
    readBatteryStatus.setValue(-1);
    notifyLowLevelBattery.setValue(0);

    BLE.advertise();
    Serial.println("Configuração BLE concluída.");
}

void Bluetooth::updateBatteryLevelCharacteristic(BLEDevice device, BLECharacteristic characteristic) {
    int batteryLevel = batteryMonitor->getBatteryPercentage();
    Serial.print("Atualizando nível de bateria: ");
    Serial.println(batteryLevel);
    characteristic.writeValue(batteryLevel);
}

void Bluetooth::updateBatteryStatusCharacteristic(BLEDevice device, BLECharacteristic characteristic) {
    int batteryStatus = batteryMonitor->getBatteryStatus();
    Serial.print("Atualizando status da bateria: ");
    Serial.println(batteryStatus);
    characteristic.writeValue(batteryStatus);
}

void Bluetooth::onBLEConnected(BLEDevice central) {
    Serial.print("Conectado, central: ");
    Serial.println(central.address());
}

void Bluetooth::onBLEDisconnected(BLEDevice central) {
    Serial.print("Desconectado, central: ");
    Serial.println(central.address());
    BLE.advertise();
}

void Bluetooth::notifyBatteryLowLevel() {
    Serial.println("Notificando nível baixo de bateria.");
    notifyLowLevelBattery.writeValue(1);
}

void Bluetooth::loop() {
    BLE.poll();
}

// Função estática para manipular eventos de leitura de nível de bateria
void Bluetooth::updateBatteryLevelCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic) {
    if (instance != nullptr) {
        instance->updateBatteryLevelCharacteristic(device, characteristic);
    }
}

// Função estática para manipular eventos de leitura de status de bateria
void Bluetooth::updateBatteryStatusCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic) {
    if (instance != nullptr) {
        instance->updateBatteryStatusCharacteristic(device, characteristic);
    }
}
