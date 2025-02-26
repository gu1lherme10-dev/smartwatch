#include "bluetooth.h"
#include <ArduinoBLE.h>
#include <battery.h>

// Definição do serviço e características do BLE
BLEService batteryService("0x180F");
BLEIntCharacteristic readBatteryLevel("0x2A19", BLERead);
BLEIntCharacteristic readBatteryStatus("0x2BED", BLERead);
BLEByteCharacteristic notifyLowLevelBattery("0x2BE9", BLENotify);

// Variáveis de controle
bool isCentralConnected = false;

// Funções de callback para conexão e desconexão do BLE
void blePeripheralConnectHandler(BLEDevice central)
{
    isCentralConnected = true;
    Serial.print("Conectado, central: ");
    Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central)
{
    isCentralConnected = false;
    Serial.print("Desconectado, central: ");
    Serial.println(central.address());
    BLE.advertise();  // Recomeça a propagação para novos dispositivos se conectarem
}

// Função para atualizar o nível de carga da bateria no BLE
void updateBatteryLevelCharacteristic(BLEDevice central, BLECharacteristic characteristic)
{
    int batteryLevel = getBatteryPercentage();
    Serial.print("Atualizando nível de bateria: ");
    Serial.println(batteryLevel);
    if (isCentralConnected)
    {
        readBatteryLevel.writeValue(batteryLevel);
    }
}

// Função para atualizar o status da bateria no BLE
void updateBatteryStatusCharacteristic(BLEDevice central, BLECharacteristic characteristic)
{
    int batteryStatus = getBatteryStatus();
    Serial.print("Atualizando status da bateria: ");
    Serial.println(batteryStatus);
    if (isCentralConnected)
    {
        readBatteryStatus.writeValue(batteryStatus);  // Atualiza o status da bateria
    }
}

// Função chamada para notificar que a bateria está em nível baixo
void notifyBatteryLowLevel()
{
    int batteryPercentage = getBatteryPercentage();
    Serial.print("Verificando nível baixo de bateria: ");
    Serial.println(batteryPercentage);
    if (isCentralConnected && batteryPercentage < 20)  // Notifica apenas se a bateria estiver baixa
    {
        Serial.println("Notificando nível baixo de bateria.");
        notifyLowLevelBattery.writeValue(1);  // Envia o valor de notificação
    }
}

// Função para configurar o Bluetooth
void setupBluetooth()
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

    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    readBatteryLevel.setValue(-1);
    readBatteryStatus.setValue(-1);
    notifyLowLevelBattery.setValue(0);

    readBatteryLevel.setEventHandler(BLERead, updateBatteryLevelCharacteristic);
    readBatteryStatus.setEventHandler(BLERead, updateBatteryStatusCharacteristic);

    BLE.advertise();
    Serial.println("Configuração BLE concluída.");
}

void loopPollingBluetooth()
{
    BLE.poll();  // Polling para o BLE
}