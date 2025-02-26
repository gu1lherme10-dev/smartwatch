#ifndef UTILS_H
#define UTILS_H

// Declare Kalman instance here

void logMessage(String message, bool endline = true);
void logMessage(int messageInt, bool endline = true);
void initializeMovingAverage(int max_samples);
float movingMediaFilter(float sample);
#endif // UTILS_H