#include "app_controller.h"
#include "display.h"
#include "camera.h"
#include "menu.h"
#include "button.h"
#include "camera_mode.h"
#include "game_controller.h"
#include <TJpg_Decoder.h>

// Global camera availability flag
extern bool cameraAvailable;

void app_init() {
    Serial.begin(115200);
    
    // Initialize display first
    display_init();
    
    // Initialize menu system
    menu_init();
    
    // Show initial menu
    display_menu();
    
    // Initialize camera (but continue even if it fails)
    cameraAvailable = camera_init();
    if (!cameraAvailable) {
        Serial.println("Camera initialization failed! System will continue without camera functionality.");
        
        // Show warning briefly but don't halt the system
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
        tft.setTextSize(1);
        tft.setCursor(5, 60);
        tft.print("Camera Init Failed!");
        tft.setCursor(10, 80);
        tft.print("System continues");
        tft.setCursor(15, 100);
        tft.print("without camera");
        
        delay(3000); // Show message for 3 seconds
        display_menu(); // Return to menu
    } else {
        // Configure camera settings only if initialization succeeded
        camera_configure();
        Serial.println("Camera initialized successfully");
    }
    
    // Initialize JPEG decoder (for camera mode if available)
    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(tft_output);
    
    Serial.println("System ready - Menu mode active");
}

void app_handle_mode() {
    switch(currentMode) {
        case MODE_MENU:
            handle_menu_mode();
            break;
            
        case MODE_CAMERA:
            handle_camera_mode();
            break;
            
        case MODE_OBJECT_DETECTION:
        case MODE_AI_ASSISTANT:
            handle_object_detection_mode();
            break;
            
        case MODE_GAME_MENU:
            handle_game_menu_mode();
            break;
            
        case MODE_SNAKE:
        case MODE_PONG:
        case MODE_BREAKOUT:
        case MODE_FLAPPY_BIRD:
        case MODE_MEMORY:
            handle_game_modes();
            break;
    }
}

void handle_menu_mode() {
    MenuOption selection = handle_menu_input();
    if (selection != (MenuOption)-1) {
        // User made a selection, switch modes with correct mapping
        AppMode targetMode;
        switch(selection) {
            case MENU_CAMERA:
                targetMode = MODE_CAMERA;
                break;
            case MENU_OBJECT_DETECTION:
                targetMode = MODE_OBJECT_DETECTION;
                break;
            case MENU_AI_ASSISTANT:
                targetMode = MODE_AI_ASSISTANT;
                break;
            case MENU_GAME:
                targetMode = MODE_GAME;
                break;
            default:
                targetMode = MODE_MENU;
                break;
        }
        run_selected_mode(targetMode);
    }
}

void handle_object_detection_mode() {
    // Check for back button to return to menu
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to menu...");
        currentMode = MODE_MENU;
        display_menu();
    }
    // Add specific functionality for each mode here in the future
    delay(100); // Small delay to prevent excessive CPU usage
}

void handle_ai_assistant_mode() {
    handle_object_detection_mode(); // Same behavior for now
}

void handle_game_menu_mode() {
    GameOption gameSelection = handle_game_menu_input();
    if (gameSelection != (GameOption)-1) {
        // User selected a game, switch to game mode
        AppMode targetGameMode;
        switch(gameSelection) {
            case GAME_SNAKE:
                targetGameMode = MODE_SNAKE;
                break;
            case GAME_PONG:
                targetGameMode = MODE_PONG;
                break;
            case GAME_BREAKOUT:
                targetGameMode = MODE_BREAKOUT;
                break;
            case GAME_FLAPPY_BIRD:
                targetGameMode = MODE_FLAPPY_BIRD;
                break;
            case GAME_MEMORY:
                targetGameMode = MODE_MEMORY;
                break;
            default:
                targetGameMode = MODE_GAME_MENU;
                break;
        }
        run_selected_game(targetGameMode);
    }
}
