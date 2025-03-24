#include "BatteryService.h"

BatteryBLEService* BatteryBLEService::instance = nullptr;

BatteryBLEService::BatteryBLEService(BatteryMonitor* batteryMonitor)
    : batteryMonitor(batteryMonitor),
      batteryService("0x180F"),
      readBatteryLevel("0x2A19", BLERead),
      readBatteryStatus("0x2BED", BLERead),
      notifyLowLevelBattery("0x2BE9", BLENotify) {}

void BatteryBLEService::begin() {
    batteryService.addCharacteristic(readBatteryLevel);
    batteryService.addCharacteristic(readBatteryStatus);
    batteryService.addCharacteristic(notifyLowLevelBattery);
    BLE.addService(batteryService);

    readBatteryLevel.setEventHandler(BLERead, updateBatteryLevelCharacteristicStatic);
    readBatteryStatus.setEventHandler(BLERead, updateBatteryStatusCharacteristicStatic);

    readBatteryLevel.setValue(0);
    readBatteryStatus.setValue(0);
    notifyLowLevelBattery.setValue(0);
}

void BatteryBLEService::loop() {}

void BatteryBLEService::updateBatteryLevelCharacteristic(BLEDevice central, BLECharacteristic characteristic) {
    int batteryLevel = batteryMonitor->getBatteryPercentage();
    characteristic.writeValue(batteryLevel);
    Serial.println("Leitura do nível de bateria");
}

void BatteryBLEService::updateBatteryStatusCharacteristic(BLEDevice device, BLECharacteristic characteristic) {
    int batteryStatus = batteryMonitor->getBatteryStatus();
    Serial.print("Atualizando status da bateria: ");
    Serial.println(batteryStatus);
    characteristic.writeValue(batteryStatus);
}

void BatteryBLEService::notifyBatteryLowLevel() {
    Serial.println("Notificando nível baixo de bateria.");
    notifyLowLevelBattery.writeValue(1);
}

void BatteryBLEService::updateBatteryLevelCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic) {
    if (instance != nullptr) {
        instance->updateBatteryLevelCharacteristic(device, characteristic);
    }
}

void BatteryBLEService::updateBatteryStatusCharacteristicStatic(BLEDevice device, BLECharacteristic characteristic) {
    if (instance != nullptr) {
        instance->updateBatteryStatusCharacteristic(device, characteristic);
    }
}