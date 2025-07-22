#include "games.h"
#include "../display.h"
#include "../menu.h"

// Flappy Bird Game Variables
static int birdY = 80;
static int birdVelocity = 0;
static int pipeX = 128;
static int pipeGapY = 60; // Center of gap
static int flappyScore = 0;
static bool gameStarted = false;

void init_flappy_bird_game() {
    tft.fillScreen(ST7735_BLACK);
    birdY = 80;
    birdVelocity = 0;
    pipeX = 128;
    pipeGapY = random(40, 100); // Random gap position
    flappyScore = 0;
    gameStarted = false;
    
    // Draw score
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print("FLAPPY - Score: ");
    tft.print(flappyScore);
    
    // Draw start message
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(15, 75);
    tft.print("Press UP to start");
    
    Serial.println("Flappy Bird game initialized");
}

void update_flappy_bird_game() {
    // Handle input
    if (button_pressed(BTN_UP)) {
        gameStarted = true;
        birdVelocity = -4; // Jump
    }
    
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to game menu from Flappy Bird");
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    if (!gameStarted) {
        return; // Wait for start
    }
    
    // Physics
    birdVelocity += 1; // Gravity
    birdY += birdVelocity;
    
    // Move pipe
    pipeX -= 3;
    if (pipeX < -20) {
        pipeX = 128;
        pipeGapY = random(40, 100);
        flappyScore++;
        
        // Update score
        tft.fillRect(0, 0, 128, 15, ST7735_BLACK);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(1);
        tft.setCursor(5, 5);
        tft.print("FLAPPY - Score: ");
        tft.print(flappyScore);
    }
    
    // Check collision with ground/ceiling
    if (birdY < 16 || birdY > 150) {
        // Game over
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_RED, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(20, 60);
        tft.print("GAME OVER");
        tft.setTextSize(1);
        tft.setCursor(30, 85);
        tft.print("Score: ");
        tft.print(flappyScore);
        
        // Non-blocking game over - auto return after 3 seconds
        delay(3000);
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    // Check collision with pipes
    int gapSize = 30;
    if (pipeX < 20 && pipeX > -20) { // Bird is at pipe position
        if (birdY < pipeGapY - gapSize/2 || birdY > pipeGapY + gapSize/2) {
            // Game over
            tft.fillScreen(ST7735_BLACK);
            tft.setTextColor(ST7735_RED, ST7735_BLACK);
            tft.setTextSize(2);
            tft.setCursor(20, 60);
            tft.print("GAME OVER");
            tft.setTextSize(1);
            tft.setCursor(30, 85);
            tft.print("Score: ");
            tft.print(flappyScore);
            
            // Non-blocking game over - auto return after 3 seconds
            delay(3000);
            currentMode = MODE_GAME_MENU;
            display_game_menu();
            return;
        }
    }
    
    // Draw game
    tft.fillRect(0, 16, 128, 144, ST7735_BLUE); // Sky
    
    // Draw pipes
    tft.fillRect(pipeX, 16, 20, pipeGapY - gapSize/2 - 16, ST7735_GREEN); // Top pipe
    tft.fillRect(pipeX, pipeGapY + gapSize/2, 20, 144 - (pipeGapY + gapSize/2), ST7735_GREEN); // Bottom pipe
    
    // Draw bird
    tft.fillRect(18, birdY, 6, 6, ST7735_YELLOW);
}
