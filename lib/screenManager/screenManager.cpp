#include "ScreenManager.h"

ScreenManager::ScreenManager(TTGOClass *watch) {
    this->watch = watch;
    this->tft = watch->tft;
    this->screenOn = false; 
}

void ScreenManager::updateBattery(int batteryPercentage) {
    if (!screenOn) return;  // Só atualiza a bateria se a tela estiver ligada
    tft->fillRect(40, 100, 200, 40, TFT_BLACK); 
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(45, 118);
    tft->print("Bateria: ");
    tft->print(batteryPercentage);
    tft->print("%");
}

void ScreenManager::showStepCount(uint32_t steps) {
    if (!screenOn) return;
    tft->fillRect(40, 140, 200, 40, TFT_BLACK); 
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(45, 158);
    tft->print("Passos: ");
    tft->print(steps);
}

void ScreenManager::turnOn() {
    watch->openBL();  // Liga o backlight
    screenOn = true;
}

bool ScreenManager::isPressed() {
    return watch->touch->getTouched();
}

bool ScreenManager::screenTimeout() {
    static unsigned long lastInteraction = millis();
    if (isPressed()) {
        lastInteraction = millis(); 
    }
    return (millis() - lastInteraction) > 15000;
}

void ScreenManager::turnOff() {
    watch->closeBL();  // Desliga o backlight
    screenOn = false;
}

bool ScreenManager::isOn() {
    return screenOn;
}

void ScreenManager::displayActivitySummary(PhysicalActivity::ActivitySummary summary) {
    // Exibindo o resumo das atividades na tela
    tft->fillScreen(TFT_BLACK);
    tft->setTextSize(2);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);

    int yPosition = 10;
    tft->setCursor(10, yPosition);
    tft->print("Resumo do Dia:");

    // Exibindo os totais das atividades
    yPosition += 30;
    tft->setCursor(10, yPosition);
    tft->print("Parado: ");
    tft->print(summary.idleTime, 1);
    tft->print(" horas");

    yPosition += 30;
    tft->setCursor(10, yPosition);
    tft->print("Caminhando: ");
    tft->print(summary.walkingTime, 1);
    tft->print(" horas");

    yPosition += 30;
    tft->setCursor(10, yPosition);
    tft->print("Correndo: ");
    tft->print(summary.runningTime, 1);
    tft->print(" horas");
}
