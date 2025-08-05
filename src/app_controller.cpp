#include "app_controller.h"
#include "display.h"
#include "camera.h"
#include "menu.h"
#include "button.h"
#include "camera_mode.h"
#include "game_controller.h"
#include "sd_card.h"
#include "photos_mode.h"
#include "face_recognition.h"
#include "ai_assistant_mode.h"
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
    
    // Initialize SD card for photo storage
    Serial.println("=== SD CARD INITIALIZATION ===");
    sd_card_test_pins(); // Test pin connectivity first
    
    if (sd_card_init()) {
        Serial.println("SD card initialized successfully");
        sd_card_debug_info(); // Show detailed info
    } else {
        Serial.println("SD card initialization failed! Running diagnostics...");
        sd_card_test_connection();
        sd_card_debug_info();
        Serial.println("Photos mode will show error message.");
    }
    
    Serial.println("System ready - Menu mode active");
    
    // Initialize AI Assistant (but don't activate it yet)
    ai_assistant_init();
}

void app_handle_mode() {
    switch(currentMode) {
        case MODE_MENU:
            handle_menu_mode();
            break;
            
        case MODE_CAMERA:
            handle_camera_mode();
            break;
            
        case MODE_PHOTOS:
            handle_photos_mode();
            break;
            
        case MODE_FACE_RECOGNITION_MENU:
            handle_face_recognition_menu_mode();
            break;
            
        case MODE_FACE_ENROLL:
            handle_face_enroll_mode();
            break;
            
        case MODE_FACE_RECOGNIZE:
            handle_face_recognize_mode();
            break;
            
        case MODE_OBJECT_DETECTION:
        case MODE_AI_ASSISTANT:
            handle_ai_assistant_mode();
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
    // Debug trigger: Hold UP + DOWN + SELECT for 2 seconds
    static unsigned long debug_hold_start = 0;
    bool up_button_current = button_pressed(BTN_UP);
    bool down_button_current = button_pressed(BTN_DOWN);
    bool select_button_current = button_pressed(BTN_SELECT);
    
    if (up_button_current && down_button_current && select_button_current) {
        if (debug_hold_start == 0) {
            debug_hold_start = millis();
            Serial.println("Debug trigger detected - hold for 2 seconds...");
        } else if (millis() - debug_hold_start > 2000) {
            Serial.println("\n=== MANUAL SD CARD DEBUG TRIGGERED ===");
            sd_card_debug_info();
            sd_card_test_pins();
            sd_card_test_connection();
            Serial.println("=== DEBUG COMPLETE ===\n");
            debug_hold_start = 0; // Reset
            return; // Skip menu processing this cycle
        }
    } else {
        debug_hold_start = 0; // Reset if not all buttons pressed
    }
    
    // Normal menu processing
    MenuOption selection = handle_menu_input();
    if (selection != (MenuOption)-1) {
        // User made a selection, switch modes with correct mapping
        AppMode targetMode;
        switch(selection) {
            case MENU_CAMERA:
                targetMode = MODE_CAMERA;
                break;
            case MENU_PHOTOS:  // Add Photos case
                targetMode = MODE_PHOTOS;
                break;
            case MENU_FACE_RECOGNITION:
                targetMode = MODE_FACE_RECOGNITION_MENU;
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
    // Initialize AI Assistant when first entering
    static bool ai_initialized = false;
    if (!ai_initialized) {
        ai_assistant_enter();
        ai_initialized = true;
    }
    
    // Update AI Assistant
    ai_assistant_update();
    
    // Reset initialization flag when leaving mode
    if (currentMode != MODE_AI_ASSISTANT) {
        ai_initialized = false;
    }
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

void handle_face_recognition_menu_mode() {
    FaceRecognitionMenuOption selection = handle_face_recognition_menu_input();
    if (selection != (FaceRecognitionMenuOption)-1) {
        AppMode targetMode;
        switch(selection) {
            case FACE_MENU_ENROLL:
                targetMode = MODE_FACE_ENROLL;
                break;
            case FACE_MENU_RECOGNIZE:
                targetMode = MODE_FACE_RECOGNIZE;
                break;
            case FACE_MENU_VIEW_DATABASE:
                // For now, just show the database info on the current screen
                show_face_database_info();
                return; // Stay in face menu mode
            case FACE_MENU_DELETE_ALL:
                // Show confirmation and delete if confirmed
                if (confirm_delete_all_faces()) {
                    delete_all_faces();
                }
                return; // Stay in face menu mode
            case FACE_MENU_BACK:
            default:
                targetMode = MODE_MENU;
                break;
        }
        currentMode = targetMode;
        if (targetMode == MODE_MENU) {
            display_menu();
        } else {
            display_face_recognition_screen(targetMode);
        }
    }
}

void handle_face_enroll_mode() {
    handle_face_enrollment();
}

void handle_face_recognize_mode() {
    handle_face_recognition();
}
