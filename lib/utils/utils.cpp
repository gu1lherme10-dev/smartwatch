#include <Arduino.h>
#include "utils.h"
// Function to log a String message
#define MAX_SAMPLES 10

static int j = 0;
static float sampleMedia = 0.0;
static float *movingAverageSamples = nullptr;
static int maxSamples = 0;

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
