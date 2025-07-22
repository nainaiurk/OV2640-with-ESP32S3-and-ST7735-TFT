#include "games.h"
#include "../display.h"
#include "../menu.h"

// Snake Game Variables
struct SnakeSegment {
    int x, y;
};

static SnakeSegment snake[50];
static int snakeLength = 3;
static int snakeDir = 0; // 0=right, 1=down, 2=left, 3=up
static int foodX, foodY;
static int snakeScore = 0;
static bool gameOver = false;
static unsigned long gameOverTime = 0;
static int lastDirection = -1; // Track last input to prevent multiple direction changes per frame

void init_snake_game() {
    tft.fillScreen(ST7735_BLACK);
    
    // Initialize snake
    snakeLength = 3;
    snake[0] = {64, 80}; // Head
    snake[1] = {60, 80};
    snake[2] = {56, 80};
    snakeDir = 0;
    snakeScore = 0;
    gameOver = false;
    gameOverTime = 0;
    lastDirection = -1; // Reset input tracking
    
    // Place food
    foodX = random(8, 120);
    foodY = random(16, 144);
    
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print("SNAKE - Score: ");
    tft.print(snakeScore);
    
    Serial.println("Snake game initialized");
}

void update_snake_game() {
    // Check for long press on any button to exit game
    if (button_long_pressed(BTN_UP, 2000) || button_long_pressed(BTN_DOWN, 2000) || 
        button_long_pressed(BTN_SELECT, 2000) || button_long_pressed(BTN_BACK, 2000)) {
        Serial.println("Long press detected - returning to game menu from Snake");
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    // Check if game is over and handle timeout
    if (gameOver) {
        if (millis() - gameOverTime > 3000) { // Auto return after 3 seconds
            currentMode = MODE_GAME_MENU;
            display_game_menu();
            return;
        }
        // Check for any button press to return immediately
        if (button_pressed(BTN_UP) || button_pressed(BTN_DOWN) || 
            button_pressed(BTN_SELECT) || button_pressed(BTN_BACK)) {
            currentMode = MODE_GAME_MENU;
            display_game_menu();
            return;
        }
        return; // Don't process game logic when game over
    }
    
    // Handle input - use all 4 buttons for directions, but only process once per frame
    int newDirection = -1;
    
    if (button_pressed(BTN_UP) && snakeDir != 1) {
        newDirection = 3; // Up
    } else if (button_pressed(BTN_DOWN) && snakeDir != 3) {
        newDirection = 1; // Down
    } else if (button_pressed(BTN_SELECT) && snakeDir != 2) {
        newDirection = 0; // Right (using SELECT for right)
    } else if (button_pressed(BTN_BACK) && snakeDir != 0) {
        newDirection = 2; // Left (using BACK for left)
    }
    
    // Only change direction if it's different from the last input
    if (newDirection != -1 && newDirection != lastDirection) {
        snakeDir = newDirection;
        lastDirection = newDirection;
    }
    
    // Reset direction tracking if no button is pressed
    if (!button_pressed(BTN_UP) && !button_pressed(BTN_DOWN) && 
        !button_pressed(BTN_SELECT) && !button_pressed(BTN_BACK)) {
        lastDirection = -1;
    }
    
    // Move snake
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i-1];
    }
    
    // Move head
    switch(snakeDir) {
        case 0: snake[0].x += 4; break; // Right
        case 1: snake[0].y += 4; break; // Down  
        case 2: snake[0].x -= 4; break; // Left
        case 3: snake[0].y -= 4; break; // Up
    }
    
    // Wall collision
    if (snake[0].x < 0 || snake[0].x > 124 || snake[0].y < 16 || snake[0].y > 156) {
        // Game over
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_RED, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(20, 60);
        tft.print("GAME OVER");
        tft.setTextSize(1);
        tft.setCursor(30, 85);
        tft.print("Score: ");
        tft.print(snakeScore);
        
        gameOver = true;
        gameOverTime = millis();
        return;
    }
    
    // Self collision
    for (int i = 1; i < snakeLength; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            // Game over
            tft.fillScreen(ST7735_BLACK);
            tft.setTextColor(ST7735_RED, ST7735_BLACK);
            tft.setTextSize(2);
            tft.setCursor(20, 60);
            tft.print("GAME OVER");
            tft.setTextSize(1);
            tft.setCursor(30, 90);
            tft.print("Score: ");
            tft.print(snakeScore);
            
            gameOver = true;
            gameOverTime = millis();
            return;
        }
    }
    
    // Check food
    if (abs(snake[0].x - foodX) < 4 && abs(snake[0].y - foodY) < 4) {
        snakeScore++;
        snakeLength++;
        
        // New food position
        foodX = random(8, 120);
        foodY = random(16, 144);
        
        // Update score
        tft.fillRect(0, 0, 128, 15, ST7735_BLACK);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(1);
        tft.setCursor(5, 5);
        tft.print("SNAKE - Score: ");
        tft.print(snakeScore);
    }
    
    // Clear game area
    tft.fillRect(0, 16, 128, 144, ST7735_BLACK);
    
    // Draw snake
    for (int i = 0; i < snakeLength; i++) {
        if (i == 0) {
            tft.fillRect(snake[i].x, snake[i].y, 4, 4, ST7735_GREEN); // Head
        } else {
            tft.fillRect(snake[i].x, snake[i].y, 4, 4, ST7735_YELLOW); // Body
        }
    }
    
    // Draw food
    tft.fillRect(foodX, foodY, 4, 4, ST7735_RED);
}
