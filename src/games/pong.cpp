        #include "games.h"
#include "../display.h"
#include "../menu.h"

// Pong Game Variables  
static int paddleX = 56;
static int ballX = 64;
static int ballY = 80;
static int ballDX = 2;
static int ballDY = 2;
static int pongScore = 0;

void init_pong_game() {
    tft.fillScreen(ST7735_BLACK);
    
    // Reset game variables
    paddleX = 56;
    ballX = 64;
    ballY = 80;
    ballDX = 2;
    ballDY = 2;
    pongScore = 0;
    
    // Draw score
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print("PONG - Score: ");
    tft.print(pongScore);
    
    // Draw center line (horizontal for vertical game)
    for (int x = 0; x < 128; x += 4) {
        tft.drawPixel(x, 80, ST7735_BLUE);
    }
    
    Serial.println("Vertical Pong game initialized");
}

void update_pong_game() {
    // Handle input (left/right paddle movement)
    if (button_pressed(BTN_UP) && paddleX > 8) {
        paddleX -= 8; // Move left
    }
    if (button_pressed(BTN_DOWN) && paddleX < 100) {
        paddleX += 8; // Move right
    }
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to game menu from Pong");
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    // Move ball
    ballX += ballDX;
    ballY += ballDY;
    
    // Ball collision with left/right walls
    if (ballX <= 2 || ballX >= 124) {
        ballDX = -ballDX;
    }
    
    // Ball collision with top wall
    if (ballY <= 16) {
        ballDY = -ballDY;
    }
    
    // Ball collision with paddle
    if (ballY >= 147 && ballX >= paddleX && ballX <= paddleX + 20) {
        ballDY = -ballDY;
        pongScore++;
        
        // Update score
        tft.fillRect(0, 0, 128, 15, ST7735_BLACK);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(1);
        tft.setCursor(5, 5);
        tft.print("PONG - Score: ");
        tft.print(pongScore);
    }
    
    // Game over if ball goes off bottom
    if (ballY > 160) {
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_RED, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(20, 60);
        tft.print("GAME OVER");
        tft.setTextSize(1);
        tft.setCursor(30, 85);
        tft.print("Score: ");
        tft.print(pongScore);
        
        // Non-blocking game over - auto return after 3 seconds
        delay(3000);
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    // Draw everything
    tft.fillRect(0, 20, 128, 140, ST7735_BLACK); // Clear game area
    
    // Draw paddle
    tft.fillRect(paddleX, 147, 20, 3, ST7735_WHITE);
    
    // Draw ball
    tft.fillRect(ballX, ballY, 4, 4, ST7735_WHITE);
}
                
