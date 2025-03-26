#include <Arduino.h>
#include <time.h> // Para usar struct tm e mktime
#include "utils.h"
#include "config.h"
// Function to log a String message
#define MAX_SAMPLES 10

static int j = 0;
static float sampleMedia = 0.0;
static float *movingAverageSamples = nullptr;
static int maxSamples = 0;


uint32_t convertToUnixTimestamp(const RTC_Date &date) {
    struct tm timeinfo;

    // Preenche a estrutura tm com os valores de RTC_Date
    timeinfo.tm_year = date.year - 1900; // Ano desde 1900
    timeinfo.tm_mon = date.month - 1;    // Mês (0-11)
    timeinfo.tm_mday = date.day;         // Dia do mês
    timeinfo.tm_hour = date.hour;        // Hora
    timeinfo.tm_min = date.minute;       // Minuto
    timeinfo.tm_sec = date.second;       // Segundo

    // Converte para Unix Timestamp (segundos desde 1º de janeiro de 1970)
    return mktime(&timeinfo);
}

void logMessage(String message, bool endline)
{
    if (Serial)
    {
        if (endline)
        {
            Serial.println(message);
        }
        else
        {
            Serial.print(message);
        }
    }
}

// Function to log an int message
void logMessage(int messageInt, bool endline)
{
    if (Serial)
    {
        String message = String(messageInt);
        if (endline)
        {
            Serial.println(message);
        }
        else
        {
            Serial.print(message);
        }
    }
}


void initializeMovingAverage(int max_samples)
{
    maxSamples = max_samples;
    movingAverageSamples = new float[maxSamples](); // Dynamically allocate array
}

float movingMediaFilter(float sample)
{
    // If the buffer is full, remove the oldest sample
    if (j >= maxSamples)
    {
        sampleMedia -= movingAverageSamples[j % maxSamples]; // Subtract the oldest sample
    }

    // Add the new sample to the buffer and the sum
    movingAverageSamples[j % maxSamples] = sample;
    sampleMedia += sample;

    // Increment the index
    j++;

    // Return the average, limited by the number of samples
    int count = (j < maxSamples) ? j : maxSamples;
    return sampleMedia / count;
}
