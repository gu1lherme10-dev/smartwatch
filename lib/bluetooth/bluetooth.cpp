// Bluetooth.cpp
#include "bluetooth.h"

Bluetooth* Bluetooth::instance = nullptr;

Bluetooth::Bluetooth() {
    instance = this;
}

void Bluetooth::begin() {
    Serial.println("Inicializando Bluetooth...");
    if (!BLE.begin()) {
        Serial.println("Falha ao inicializar o BLE!");
        return;
    }

    BLE.setLocalName("Smartwatch");
    BLE.setEventHandler(BLEConnected, onBLEConnected);
    BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

    for (auto* service : services) {
        service->begin();
    }

    BLE.advertise();
    Serial.println("BLE pronto!");
}

void Bluetooth::loop() {
    BLE.poll();
}

void Bluetooth::addService(BLEServiceBase* service) {
    services.push_back(service);
}

void Bluetooth::onBLEConnected(BLEDevice central) {
    Serial.print("Conectado: ");
    Serial.println(central.address());
}

void Bluetooth::onBLEDisconnected(BLEDevice central) {
    Serial.print("Desconectado: ");
    Serial.println(central.address());
    BLE.advertise();
}
