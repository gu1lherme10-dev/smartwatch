#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <config.h> // Para RTC_Date

void logMessage(String message, bool endline = true);
void logMessage(int messageInt, bool endline = true);
void initializeMovingAverage(int max_samples);
float movingMediaFilter(float sample);
uint32_t convertToUnixTimestamp(const RTC_Date &date); // Nova função para converter datetime para timestamp

#endif // UTILS_H