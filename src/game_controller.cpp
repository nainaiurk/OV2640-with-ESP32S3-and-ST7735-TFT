#include "game_controller.h"
#include "games/games.h"

void handle_game_modes() {
    switch(currentMode) {
        case MODE_SNAKE:
            run_snake_mode();
            break;
            
        case MODE_PONG:
            run_pong_mode();
            break;
            
        case MODE_BREAKOUT:
            run_breakout_mode();
            break;
            
        case MODE_FLAPPY_BIRD:
            run_flappy_bird_mode();
            break;
            
        case MODE_MEMORY:
            run_memory_mode();
            break;
            
        default:
            // Should not reach here, but handle gracefully
            currentMode = MODE_MENU;
            break;
    }
}

void run_snake_mode() {
    update_snake_game();
    delay(150); // Game speed control
}

void run_pong_mode() {
    update_pong_game();
    delay(50); // Game speed control
}

void run_breakout_mode() {
    update_breakout_game();
    delay(50); // Game speed control
}

void run_flappy_bird_mode() {
    update_flappy_bird_game();
    delay(80); // Game speed control
}

void run_memory_mode() {
    update_memory_game();
    // No delay needed, handled internally
}
