#include "button.h"

// Static variables for simple button handling
static bool lastButtonState[4] = {HIGH, HIGH, HIGH, HIGH};
static bool buttonPressed[4] = {false, false, false, false};

void button_init() {
    // Initialize button pins with input pullup
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    
    Serial.println("Button system initialized");
    Serial.println("Controls: IO4=UP, IO5=DOWN, IO14=SELECT, IO13=BACK");
}

// Get button index for array access
int get_button_index(int pin) {
    switch(pin) {
        case BTN_UP: return 0;
        case BTN_DOWN: return 1;
        case BTN_SELECT: return 2;
        case BTN_BACK: return 3;
        default: return -1;
    }
}

// Simple and responsive button press detection
bool button_pressed(int pin) {
    // Direct read - no edge detection, just check if button is currently pressed
    return (digitalRead(pin) == LOW);
}

// Simple long press detection
bool button_long_pressed(int pin, unsigned long duration) {
    static unsigned long pressStartTime[4] = {0, 0, 0, 0};
    static bool longPressTriggered[4] = {false, false, false, false};
    
    int index = get_button_index(pin);
    if (index < 0) return false;
    
    bool currentState = digitalRead(pin);
    
    if (currentState == LOW) { // Button is pressed
        if (pressStartTime[index] == 0) {
            pressStartTime[index] = millis();
            longPressTriggered[index] = false;
        }
        
        if (!longPressTriggered[index] && (millis() - pressStartTime[index]) >= duration) {
            longPressTriggered[index] = true;
            return true;
        }
    } else { // Button is released
        pressStartTime[index] = 0;
        longPressTriggered[index] = false;
    }
    
    return false;
}
