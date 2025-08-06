#include "menu.h"
#include "display.h"
#include "button.h"
#include "sd_card.h"
#include "photos_mode.h"
#include "ai_assistant_mode.h"

// Global variables
AppMode currentMode = MODE_MENU;
MenuOption selectedOption = MENU_CAMERA;
GameOption selectedGame = GAME_SNAKE;
int menuScrollOffset = 0;       // Scroll offset for main menu
int gameScrollOffset = 0;       // Scroll offset for game menu

// Menu option names
const char* menuOptions[] = {
    "Camera",
    "Photos",
    "Face Recognition",
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
    
    // Calculate scroll bounds
    int maxScroll = max(0, MENU_COUNT - VISIBLE_MENU_ITEMS);
    menuScrollOffset = constrain(menuScrollOffset, 0, maxScroll);
    
    // Auto-scroll to keep selected item visible
    if (selectedOption < menuScrollOffset) {
        menuScrollOffset = selectedOption;
    } else if (selectedOption >= menuScrollOffset + VISIBLE_MENU_ITEMS) {
        menuScrollOffset = selectedOption - VISIBLE_MENU_ITEMS + 1;
    }
    
    // Draw visible menu options with scrolling
    tft.setTextSize(1);
    
    for (int i = 0; i < VISIBLE_MENU_ITEMS && (i + menuScrollOffset) < MENU_COUNT; i++) {
        int menuIndex = i + menuScrollOffset;
        int y = 45 + (i * 22);
        
        // Clear the entire line first using actual width
        tft.fillRect(0, y-3, width, 20, ST7735_BLACK);
        
        // Highlight selected option
        if (menuIndex == selectedOption) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(menuOptions[menuIndex]);
    }
    
    // Fill any remaining screen area with black using actual dimensions
    int remainingY = 45 + (VISIBLE_MENU_ITEMS * 22);
    if (remainingY < height) {
        tft.fillRect(0, remainingY, width, height - remainingY, ST7735_BLACK);
    }
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
    
    // Calculate scroll bounds
    int maxScroll = max(0, GAME_COUNT - VISIBLE_GAME_ITEMS);
    gameScrollOffset = constrain(gameScrollOffset, 0, maxScroll);
    
    // Auto-scroll to keep selected item visible
    if (selectedGame < gameScrollOffset) {
        gameScrollOffset = selectedGame;
    } else if (selectedGame >= gameScrollOffset + VISIBLE_GAME_ITEMS) {
        gameScrollOffset = selectedGame - VISIBLE_GAME_ITEMS + 1;
    }
    
    // Draw visible game options with scrolling
    tft.setTextSize(1);
    
    for (int i = 0; i < VISIBLE_GAME_ITEMS && (i + gameScrollOffset) < GAME_COUNT; i++) {
        int gameIndex = i + gameScrollOffset;
        int y = 45 + (i * 22);
        
        // Clear the entire line first
        tft.fillRect(0, y-3, width, 20, ST7735_BLACK);
        
        // Highlight selected option
        if (gameIndex == selectedGame) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(gameOptions[gameIndex]);
    }
    
    // Fill remaining screen area with black
    int remainingY = 45 + (VISIBLE_GAME_ITEMS * 22);
    if (remainingY < height) {
        tft.fillRect(0, remainingY, width, height - remainingY, ST7735_BLACK);
    }
}

// Update only the game menu selection (smooth navigation without flicker)
void update_game_menu_selection() {
    int width = tft.width();
    
    // Calculate scroll bounds
    int maxScroll = max(0, GAME_COUNT - VISIBLE_GAME_ITEMS);
    int oldScrollOffset = gameScrollOffset;
    
    // Auto-scroll to keep selected item visible
    if (selectedGame < gameScrollOffset) {
        gameScrollOffset = selectedGame;
    } else if (selectedGame >= gameScrollOffset + VISIBLE_GAME_ITEMS) {
        gameScrollOffset = selectedGame - VISIBLE_GAME_ITEMS + 1;
    }
    
    // If scrolling occurred, just shift the menu items without full redraw
    if (oldScrollOffset != gameScrollOffset) {
        // Clear menu area only
        tft.fillRect(0, 42, width, VISIBLE_GAME_ITEMS * 22 + 6, ST7735_BLACK);
    }
    
    // Draw visible game items
    tft.setTextSize(1);
    
    for (int i = 0; i < VISIBLE_GAME_ITEMS && (i + gameScrollOffset) < GAME_COUNT; i++) {
        int gameIndex = i + gameScrollOffset;
        int y = 45 + (i * 22);
        
        // Clear the line area only if no scrolling occurred
        if (oldScrollOffset == gameScrollOffset) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLACK);
        }
        
        // Highlight selected option
        if (gameIndex == selectedGame) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(gameOptions[gameIndex]);
    }
}

// Update only the menu selection (smooth navigation without flicker)
void update_menu_selection() {
    int width = tft.width();
    
    // Calculate scroll bounds
    int maxScroll = max(0, MENU_COUNT - VISIBLE_MENU_ITEMS);
    int oldScrollOffset = menuScrollOffset;
    
    // Auto-scroll to keep selected item visible
    if (selectedOption < menuScrollOffset) {
        menuScrollOffset = selectedOption;
    } else if (selectedOption >= menuScrollOffset + VISIBLE_MENU_ITEMS) {
        menuScrollOffset = selectedOption - VISIBLE_MENU_ITEMS + 1;
    }
    
    // If scrolling occurred, just shift the menu items without full redraw
    if (oldScrollOffset != menuScrollOffset) {
        // Clear menu area only
        tft.fillRect(0, 42, width, VISIBLE_MENU_ITEMS * 22 + 6, ST7735_BLACK);
    }
    
    // Draw visible menu items
    tft.setTextSize(1);
    
    for (int i = 0; i < VISIBLE_MENU_ITEMS && (i + menuScrollOffset) < MENU_COUNT; i++) {
        int menuIndex = i + menuScrollOffset;
        int y = 45 + (i * 22);
        
        // Clear the line area only if no scrolling occurred
        if (oldScrollOffset == menuScrollOffset) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLACK);
        }
        
        // Highlight selected option
        if (menuIndex == selectedOption) {
            tft.fillRect(2, y-2, width-4, 18, ST7735_BLUE);
            tft.setTextColor(ST7735_WHITE, ST7735_BLUE);
            tft.setCursor(8, y);
            tft.print("> ");
        } else {
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setCursor(8, y);
            tft.print("  ");
        }
        
        tft.print(menuOptions[menuIndex]);
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
        tft.setTextSize(1); // Reduced from size 2 to size 1 for better fit
        
        // Center "Starting" text
        String startingText = "Starting";
        int startingWidth = startingText.length() * 6;
        int startingX = (tft.width() - startingWidth) / 2;
        tft.setCursor(startingX, 60);
        tft.print(startingText);
        
        // Center the mode name
        String modeName = String(menuOptions[selectedOption]);
        int modeWidth = modeName.length() * 6;
        int modeX = (tft.width() - modeWidth) / 2;
        tft.setCursor(modeX, 85);
        tft.print(modeName);
        
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
        tft.setTextSize(1); // Reduced from size 2 to size 1 for better fit
        
        // Center "Starting" text
        String startingText = "Starting";
        int startingWidth = startingText.length() * 6;
        int startingX = (tft.width() - startingWidth) / 2;
        tft.setCursor(startingX, 60);
        tft.print(startingText);
        
        // Center the game name
        String gameName = String(gameOptions[selectedGame]);
        int gameWidth = gameName.length() * 6;
        int gameX = (tft.width() - gameWidth) / 2;
        tft.setCursor(gameX, 85);
        tft.print(gameName);
        
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
            
        case MODE_PHOTOS:
            currentMode = MODE_PHOTOS;
            photos_mode_init();
            Serial.println("Photos mode activated");
            break;
            
        case MODE_OBJECT_DETECTION:
        {
            currentMode = MODE_OBJECT_DETECTION;
            tft.fillScreen(ST7735_BLACK);
            tft.fillRect(0, 0, 128, 160, ST7735_BLACK); // Full coverage
            tft.setTextColor(ST7735_RED, ST7735_BLACK);
            tft.setTextSize(1);
            
            // Center "Object Detection" text
            String objDetText = "Object Detection";
            int objDetWidth = objDetText.length() * 6;
            int objDetX = (tft.width() - objDetWidth) / 2;
            tft.setCursor(objDetX, 70);
            tft.print(objDetText);
            
            // Center "Coming Soon!" text
            String comingSoonText = "Coming Soon!";
            int comingSoonWidth = comingSoonText.length() * 6;
            int comingSoonX = (tft.width() - comingSoonWidth) / 2;
            tft.setCursor(comingSoonX, 90);
            tft.print(comingSoonText);
            Serial.println("Object Detection mode (placeholder)");
            break;
        }
            
        case MODE_AI_ASSISTANT:
        {
            currentMode = MODE_AI_ASSISTANT;
            Serial.println("Entering mode: AI Assistant");
            break;
        }
            
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
