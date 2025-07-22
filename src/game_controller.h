#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <Arduino.h>
#include "menu.h"

// Function declarations for game coordination
void handle_game_modes();
void run_snake_mode();
void run_pong_mode();
void run_breakout_mode();
void run_flappy_bird_mode();
void run_memory_mode();

#endif
