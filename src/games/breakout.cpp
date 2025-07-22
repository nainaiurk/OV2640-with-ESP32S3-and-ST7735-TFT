#include "games.h"
#include "../display.h"
#include "../menu.h"

// Breakout Game Variables
static int paddleX = 50;
static int ballX = 64, ballY = 100;
static int ballDX = 2, ballDY = -2;
static int breakoutScore = 0;
static bool bricks[8][6]; // 8 columns, 6 rows
static int bricksRemaining = 48;

void init_breakout_game() {
    tft.fillScreen(ST7735_BLACK);
    paddleX = 50;
    ballX = 64; ballY = 100;
    ballDX = 2; ballDY = -2;
    breakoutScore = 0;
    bricksRemaining = 48;
    
    // Initialize bricks
    for (int row = 0; row < 6; row++) {
        for (int col = 0; col < 8; col++) {
            bricks[col][row] = true;
        }
    }
    
    // Draw score
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print("BREAKOUT - Score: ");
    tft.print(breakoutScore);
    
    // Draw bricks
    for (int row = 0; row < 6; row++) {
        for (int col = 0; col < 8; col++) {
            if (bricks[col][row]) {
                int x = col * 16;
                int y = 20 + row * 8;
                uint16_t color = ST7735_RED;
                if (row < 2) color = ST7735_RED;
                else if (row < 4) color = ST7735_YELLOW;
                else color = ST7735_GREEN;
                
                tft.fillRect(x, y, 14, 6, color);
            }
        }
    }
    
    Serial.println("Breakout game initialized");
}

void update_breakout_game() {
    // Handle input
    if (button_pressed(BTN_UP) && paddleX > 8) {
        paddleX -= 8;
    }
    if (button_pressed(BTN_DOWN) && paddleX < 100) {
        paddleX += 8;
    }
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to game menu from Breakout");
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    // Move ball
    ballX += ballDX;
    ballY += ballDY;
    
    // Ball collision with walls
    if (ballX <= 0 || ballX >= 123) ballDX = -ballDX;
    if (ballY <= 16) ballDY = -ballDY;
    
    // Ball collision with paddle
    if (ballY >= 145 && ballX >= paddleX && ballX <= paddleX + 20) {
        ballDY = -ballDY;
    }
    
    // Ball collision with bricks
    int brickCol = ballX / 16;
    int brickRow = (ballY - 20) / 8;
    
    if (brickRow >= 0 && brickRow < 6 && brickCol >= 0 && brickCol < 8) {
        if (bricks[brickCol][brickRow]) {
            bricks[brickCol][brickRow] = false;
            ballDY = -ballDY;
            breakoutScore += 10;
            bricksRemaining--;
            
            // Update score
            tft.fillRect(0, 0, 128, 15, ST7735_BLACK);
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setTextSize(1);
            tft.setCursor(5, 5);
            tft.print("BREAKOUT - Score: ");
            tft.print(breakoutScore);
        }
    }
    
    // Check win condition
    if (bricksRemaining == 0) {
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(30, 60);
        tft.print("YOU WIN!");
        tft.setTextSize(1);
        tft.setCursor(30, 85);
        tft.print("Score: ");
        tft.print(breakoutScore);
        
        // Non-blocking game over - auto return after 3 seconds
        delay(3000);
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    // Game over
    if (ballY > 160) {
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_RED, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(20, 60);
        tft.print("GAME OVER");
        tft.setTextSize(1);
        tft.setCursor(30, 85);
        tft.print("Score: ");
        tft.print(breakoutScore);
        
        // Non-blocking game over - auto return after 3 seconds
        delay(3000);
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    // Draw everything
    tft.fillRect(0, 68, 128, 92, ST7735_BLACK); // Clear game area below bricks
    
    // Draw remaining bricks
    for (int row = 0; row < 6; row++) {
        for (int col = 0; col < 8; col++) {
            if (bricks[col][row]) {
                int x = col * 16;
                int y = 20 + row * 8;
                uint16_t color = ST7735_RED;
                if (row < 2) color = ST7735_RED;
                else if (row < 4) color = ST7735_YELLOW;
                else color = ST7735_GREEN;
                
                tft.fillRect(x, y, 14, 6, color);
            } else {
                // Clear destroyed brick area
                int x = col * 16;
                int y = 20 + row * 8;
                tft.fillRect(x, y, 14, 6, ST7735_BLACK);
            }
        }
    }
    
    // Draw paddle
    tft.fillRect(paddleX, 147, 20, 3, ST7735_WHITE);
    
    // Draw ball
    tft.fillRect(ballX, ballY, 4, 4, ST7735_WHITE);
}
