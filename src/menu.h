#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "button.h"
#include "games/games.h"

// Menu options
enum MenuOption {
    MENU_CAMERA = 0,
    MENU_PHOTOS,
    MENU_OBJECT_DETECTION,
    MENU_AI_ASSISTANT, 
    MENU_GAME,
    MENU_COUNT // Total number of menu items
};

// Current application mode
enum AppMode {
    MODE_MENU = 0,
    MODE_CAMERA,
    MODE_PHOTOS,
    MODE_OBJECT_DETECTION,
    MODE_AI_ASSISTANT,
    MODE_GAME,
    MODE_GAME_MENU,
    MODE_SNAKE,
    MODE_PONG,
    MODE_BREAKOUT,
    MODE_FLAPPY_BIRD,
    MODE_MEMORY
};

// Game menu options
enum GameOption {
    GAME_SNAKE = 0,
    GAME_PONG,
    GAME_BREAKOUT,
    GAME_FLAPPY_BIRD,
    GAME_MEMORY,
    GAME_COUNT // Total number of games
};

// Global variables
extern AppMode currentMode;
extern MenuOption selectedOption;
extern GameOption selectedGame;

// Function declarations
void menu_init();
void display_menu();
void update_menu_selection(); // New function for smooth navigation
void display_game_menu();
void update_game_menu_selection(); // New function for smooth game navigation
MenuOption handle_menu_input();
GameOption handle_game_menu_input();
void run_selected_mode(AppMode mode);
void run_selected_game(AppMode gameMode);

#endif
