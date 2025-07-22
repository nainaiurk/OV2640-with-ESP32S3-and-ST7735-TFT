#include "menu.h"
#include "display.h"
#include "button.h"

// Global variables
AppMode currentMode = MODE_MENU;
MenuOption selectedOption = MENU_CAMERA;
GameOption selectedGame = GAME_SNAKE;

// Menu option names
const char* menuOptions[] = {
    "Camera",
    "Object Detection", 
    "AI Assistant",
    "Game"
};

// Game menu option names
const char* gameOptions[] = {
    "Snake",
    "Pong",
    "Breakout",
    "Flappy Bird", 
    "Memory Game"
};

void menu_init() {
    button_init();
    Serial.println("Menu system initialized");
}

void display_menu() {
    // Get actual display dimensions
    int width = tft.width();
    int height = tft.height();
    
    // Ensure complete black background coverage using actual dimensions
    tft.fillScreen(ST7735_BLACK);
    
    // Manual fill to ensure complete coverage
    tft.startWrite();
    tft.setAddrWindow(0, 0, width, height);
    for (int i = 0; i < width * height; i++) {
        tft.pushColor(ST7735_BLACK);
    }
    tft.endWrite();
    
    // Draw title centered
    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
    tft.setTextSize(2);
    tft.setCursor((width - 60) / 2, 15); // Center the "MENU" text
    tft.print("MENU");
    
    // Draw menu options with full background coverage using actual dimensions
    tft.setTextSize(1);
    
    for (int i = 0; i < MENU_COUNT; i++) {
        int y = 45 + (i * 22);
        
        // Clear the entire line first using actual width
        tft.fillRect(0, y-3, width, 20, ST7735_BLACK);
        
        // Highlight selected option
        if (i == selectedOption) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(menuOptions[i]);
    }
    
    // Fill any remaining screen area with black using actual dimensions
    int remainingY = 45 + (MENU_COUNT * 22);
    if (remainingY < height) {
        tft.fillRect(0, remainingY, width, height - remainingY, ST7735_BLACK);
    }
    
    // Draw instructions at bottom in smaller font - centered and higher
    tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
    tft.setTextSize(1); // Small font for instructions
    
    // Center "<-- Up/Down" horizontally
    String upDownText = "<-- Up/Down";
    int textWidth1 = upDownText.length() * 6; // Each character is ~6 pixels wide in size 1
    int centerX1 = (width - textWidth1) / 2;
    tft.setCursor(centerX1, height - 25);
    tft.print(upDownText);
    
    // Center "Select/Back-->" horizontally  
    String selectBackText = "Select/Back-->";
    int textWidth2 = selectBackText.length() * 6;
    int centerX2 = (width - textWidth2) / 2;
    tft.setCursor(centerX2, height - 15);
    tft.print(selectBackText);
}

void display_game_menu() {
    // Get actual display dimensions
    int width = tft.width();
    int height = tft.height();
    
    // Ensure complete black background coverage
    tft.fillScreen(ST7735_BLACK);
    
    // Draw title centered
    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
    tft.setTextSize(2);
    tft.setCursor((width - 60) / 2, 15); // Center the "GAMES" text
    tft.print("GAMES");
    
    // Draw game options
    tft.setTextSize(1);
    
    for (int i = 0; i < GAME_COUNT; i++) {
        int y = 45 + (i * 22);
        
        // Clear the entire line first
        tft.fillRect(0, y-3, width, 20, ST7735_BLACK);
        
        // Highlight selected option
        if (i == selectedGame) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(gameOptions[i]);
    }
    
    // Fill remaining screen area with black
    int remainingY = 45 + (GAME_COUNT * 22);
    if (remainingY < height) {
        tft.fillRect(0, remainingY, width, height - remainingY, ST7735_BLACK);
    }
    
    // Draw instructions at bottom - removed for cleaner UI
    tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
    tft.setTextSize(1);
}

// Update only the game menu selection (smooth navigation without flicker)
void update_game_menu_selection() {
    int width = tft.width();
    
    // Redraw all game options with new selection
    tft.setTextSize(1);
    
    for (int i = 0; i < GAME_COUNT; i++) {
        int y = 45 + (i * 22);
        
        // Clear the line area
        tft.fillRect(2, y-2, width-4, 18, ST7735_BLACK);
        
        // Highlight selected option
        if (i == selectedGame) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(gameOptions[i]);
    }
}

// Update only the menu selection (smooth navigation without flicker)
void update_menu_selection() {
    int width = tft.width();
    
    // Redraw all menu options with new selection
    tft.setTextSize(1);
    
    for (int i = 0; i < MENU_COUNT; i++) {
        int y = 45 + (i * 22);
        
        // Clear the line area
        tft.fillRect(2, y-2, width-4, 18, ST7735_BLACK);
        
        // Highlight selected option
        if (i == selectedOption) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(menuOptions[i]);
    }
}

MenuOption handle_menu_input() {
    if (button_pressed(BTN_UP)) {
        selectedOption = (MenuOption)((selectedOption - 1 + MENU_COUNT) % MENU_COUNT);
        update_menu_selection(); // Use smooth update instead of full redraw
        Serial.printf("Selected: %s\n", menuOptions[selectedOption]);
    }
    
    if (button_pressed(BTN_DOWN)) {
        selectedOption = (MenuOption)((selectedOption + 1) % MENU_COUNT);
        update_menu_selection(); // Use smooth update instead of full redraw
        Serial.printf("Selected: %s\n", menuOptions[selectedOption]);
    }
    
    if (button_pressed(BTN_SELECT)) {
        Serial.printf("Entering mode: %s\n", menuOptions[selectedOption]);
        
        // Complete screen clear for selected mode
        tft.fillScreen(ST7735_BLACK);
        tft.fillRect(0, 0, 128, 160, ST7735_BLACK); // Ensure full coverage
        
        tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(15, 60);
        tft.print("Starting");
        tft.setCursor(10, 85);
        tft.print(menuOptions[selectedOption]);
        
        delay(1500); // Show message briefly
        
        // Return selected option to switch modes
        return selectedOption;
    }
    
    return (MenuOption)-1; // No selection made
}

GameOption handle_game_menu_input() {
    if (button_pressed(BTN_UP)) {
        selectedGame = (GameOption)((selectedGame - 1 + GAME_COUNT) % GAME_COUNT);
        update_game_menu_selection(); // Use smooth update instead of full redraw
        Serial.printf("Selected game: %s\n", gameOptions[selectedGame]);
    }
    
    if (button_pressed(BTN_DOWN)) {
        selectedGame = (GameOption)((selectedGame + 1) % GAME_COUNT);
        update_game_menu_selection(); // Use smooth update instead of full redraw
        Serial.printf("Selected game: %s\n", gameOptions[selectedGame]);
    }
    
    if (button_pressed(BTN_SELECT)) {
        Serial.printf("Starting game: %s\n", gameOptions[selectedGame]);
        
        // Show loading screen
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(15, 60);
        tft.print("Starting");
        tft.setCursor(10, 85);
        tft.print(gameOptions[selectedGame]);
        
        delay(1500);
        
        return selectedGame;
    }
    
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to main menu...");
        currentMode = MODE_MENU;
        display_menu();
        return (GameOption)-1;
    }
    
    return (GameOption)-1; // No selection made
}

void run_selected_mode(AppMode mode) {
    switch(mode) {
        case MODE_CAMERA:
            currentMode = MODE_CAMERA;
            Serial.println("Camera mode activated");
            break;
            
        case MODE_OBJECT_DETECTION:
            currentMode = MODE_OBJECT_DETECTION;
            tft.fillScreen(ST7735_BLACK);
            tft.fillRect(0, 0, 128, 160, ST7735_BLACK); // Full coverage
            tft.setTextColor(ST7735_RED, ST7735_BLACK);
            tft.setTextSize(1);
            tft.setCursor(15, 70);
            tft.print("Object Detection");
            tft.setCursor(25, 90);
            tft.print("Coming Soon!");
            Serial.println("Object Detection mode (placeholder)");
            break;
            
        case MODE_AI_ASSISTANT:
            currentMode = MODE_AI_ASSISTANT;
            tft.fillScreen(ST7735_BLACK);
            tft.fillRect(0, 0, 128, 160, ST7735_BLACK); // Full coverage
            tft.setTextColor(ST7735_MAGENTA, ST7735_BLACK);
            tft.setTextSize(1);
            tft.setCursor(20, 70);
            tft.print("AI Assistant");
            tft.setCursor(25, 90);
            tft.print("Coming Soon!");
            Serial.println("AI Assistant mode (placeholder)");
            break;
            
        case MODE_GAME:
            currentMode = MODE_GAME_MENU;
            display_game_menu();
            Serial.println("Game menu activated");
            break;
            
        default:
            currentMode = MODE_MENU;
            display_menu();
            break;
    }
}

void run_selected_game(AppMode gameMode) {
    switch(gameMode) {
        case MODE_SNAKE:
            currentMode = MODE_SNAKE;
            Serial.println("Snake game started");
            init_snake_game();
            break;
            
        case MODE_PONG:
            currentMode = MODE_PONG;
            Serial.println("Pong game started");
            init_pong_game();
            break;
            
        case MODE_BREAKOUT:
            currentMode = MODE_BREAKOUT;
            Serial.println("Breakout game started");
            init_breakout_game();
            break;
            
        case MODE_FLAPPY_BIRD:
            currentMode = MODE_FLAPPY_BIRD;
            Serial.println("Flappy Bird game started");
            init_flappy_bird_game();
            break;
            
        case MODE_MEMORY:
            currentMode = MODE_MEMORY;
            Serial.println("Memory game started");
            init_memory_game();
            break;
            
        default:
            currentMode = MODE_GAME_MENU;
            display_game_menu();
            break;
    }
}
