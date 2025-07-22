#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <Arduino.h>
#include "menu.h"

// Function declarations
void app_init();
void app_handle_mode();

// Mode-specific handlers
void handle_menu_mode();
void handle_camera_mode();
void handle_object_detection_mode();
void handle_ai_assistant_mode();
void handle_game_menu_mode();

#endif
