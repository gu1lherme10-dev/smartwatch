#include "bluetooth.h"
#include "battery.h"
#include "battery/BatteryService.h"
#include "physicalActivity.h"

Bluetooth bluetooth;

unsigned long lastBatteryCheck = 0;
const unsigned long batteryCheckInterval = 10000;

TTGOClass *watch;
BatteryMonitor *batteryMonitor;
PhysicalActivity *physicalActivity;
BatteryBLEService *batteryService;
TFT_eSPI *tft;

void setup() {
    Serial.begin(115200);

    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();
    tft = watch->tft;

    batteryMonitor = new BatteryMonitor(watch);
    physicalActivity = new PhysicalActivity(watch);
    batteryService = new BatteryBLEService(batteryMonitor);

    batteryMonitor->begin();
    bluetooth.begin();
    physicalActivity->begin();
    bluetooth.addService(batteryService);
    batteryService->begin();
}

void loop() {
    bluetooth.loop();

    //physicalActivity->updateActivity();

    unsigned long currentMillis = millis();
    if (currentMillis - lastBatteryCheck >= batteryCheckInterval) {
        lastBatteryCheck = currentMillis;

        if (batteryMonitor->isBatteryLowLevel() ) {
            Serial.println("Battery low level");
            batteryService->notifyBatteryLowLevel();
        }

        tft->setTextColor(random(0xFFFF), TFT_BLACK);
        tft->setCursor(45, 118);
        tft->print("Porcentagem da Bateria:");
        tft->print(batteryMonitor->getBatteryPercentage());
    }
}