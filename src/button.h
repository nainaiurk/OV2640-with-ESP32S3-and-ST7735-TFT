#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

// Button pin definitions
#define BTN_UP     4   // IO4 - Up navigation
#define BTN_DOWN   5   // IO5 - Down navigation  
#define BTN_SELECT 14  // IO14 - Select option
#define BTN_BACK   13  // IO13 - Back/Exit

// Function declarations
void button_init();
bool button_pressed(int pin);
bool button_long_pressed(int pin, unsigned long duration = 1000);

#endif
