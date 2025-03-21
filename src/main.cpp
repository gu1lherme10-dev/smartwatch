#include "bluetooth.h"
#include "battery.h"

// Instâncias globais
BatteryMonitor batteryMonitor;
Bluetooth bluetooth(&batteryMonitor);

// Variáveis de controle de tempo
unsigned long lastBatteryCheck = 0;
const unsigned long batteryCheckInterval = 10000; // 10 segundos

void setup() {
    Serial.begin(115200);
    
    batteryMonitor.begin();
    bluetooth.begin();
}

void loop() {
    batteryMonitor.loop();
    bluetooth.loop();

    unsigned long currentMillis = millis();
    if (currentMillis - lastBatteryCheck >= batteryCheckInterval) {
        lastBatteryCheck = currentMillis;

        if (batteryMonitor.isBatteryLowLevel()) {
            Serial.println("Battery low level");
            bluetooth.notifyBatteryLowLevel();
        }
    }
}
