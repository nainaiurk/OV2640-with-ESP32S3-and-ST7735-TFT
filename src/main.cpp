#include <Arduino.h>
#include "app_controller.h"

// Global camera availability flag
bool cameraAvailable = false;

void setup() {
    app_init();
}

void loop() {
    app_handle_mode();
}

