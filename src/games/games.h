#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include "../button.h"

// Game function declarations
void init_snake_game();
void update_snake_game();
void init_pong_game();
void update_pong_game();
void init_breakout_game();
void update_breakout_game();
void init_memory_game();
void update_memory_game();
void init_flappy_bird_game();
void update_flappy_bird_game();

#endif
