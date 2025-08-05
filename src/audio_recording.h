#ifndef AUDIO_RECORDING_H
#define AUDIO_RECORDING_H

#include <Arduino.h>

// Function declarations for audio recording
void I2S_Record_Init();
void Record_Start(String filename);
bool Record_Available(String filename, float* recorded_seconds);

#endif
