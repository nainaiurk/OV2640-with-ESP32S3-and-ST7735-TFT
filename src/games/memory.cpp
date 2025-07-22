#include "games.h"
#include "../display.h"
#include "../menu.h"

// Memory Game Variables
static int sequence[20];
static int sequenceLength = 1;
static int sequenceDisplayIndex = 0;
static int playerIndex = 0;
static int memoryScore = 0;
static bool showingSequence = true;
static unsigned long sequenceTimer = 0;
static uint16_t memoryColors[4] = {ST7735_RED, ST7735_GREEN, ST7735_BLUE, ST7735_YELLOW};

void init_memory_game() {
    tft.fillScreen(ST7735_BLACK);
    
    // Reset game state
    sequenceLength = 1;
    sequenceDisplayIndex = 0;
    playerIndex = 0;
    memoryScore = 0;
    showingSequence = true;
    
    // Generate initial sequence
    for (int i = 0; i < 20; i++) {
        sequence[i] = random(0, 4);
    }
    
    // Draw score
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.print("MEMORY - Score: ");
    tft.print(memoryScore);
    
    // Draw instruction
    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
    tft.setCursor(15, 25);
    tft.print("Watch sequence!");
    
    // Draw color buttons (initially dim)
    tft.fillRect(20, 50, 30, 30, ST7735_RED);
    tft.fillRect(78, 50, 30, 30, ST7735_GREEN);
    tft.fillRect(20, 100, 30, 30, ST7735_BLUE);
    tft.fillRect(78, 100, 30, 30, ST7735_YELLOW);
    
    // Draw button labels
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(25, 140);
    tft.print("U D S B");
    
    sequenceTimer = millis();
    Serial.println("Memory game initialized");
}

void update_memory_game() {
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to game menu from Memory");
        currentMode = MODE_GAME_MENU;
        display_game_menu();
        return;
    }
    
    if (showingSequence) {
        // Show sequence to player
        if (millis() - sequenceTimer > 800) {
            if (sequenceDisplayIndex < sequenceLength) {
                // First restore normal colors
                tft.fillRect(20, 50, 30, 30, ST7735_RED);
                tft.fillRect(78, 50, 30, 30, ST7735_GREEN);
                tft.fillRect(20, 100, 30, 30, ST7735_BLUE);
                tft.fillRect(78, 100, 30, 30, ST7735_YELLOW);
                
                delay(200);
                
                // Highlight current color in sequence
                int colorIndex = sequence[sequenceDisplayIndex];
                
                switch(colorIndex) {
                    case 0: tft.fillRect(20, 50, 30, 30, ST7735_WHITE); break;  // Red -> White
                    case 1: tft.fillRect(78, 50, 30, 30, ST7735_WHITE); break;  // Green -> White
                    case 2: tft.fillRect(20, 100, 30, 30, ST7735_WHITE); break; // Blue -> White
                    case 3: tft.fillRect(78, 100, 30, 30, ST7735_WHITE); break; // Yellow -> White
                }
                
                sequenceDisplayIndex++;
                sequenceTimer = millis();
            } else {
                // Sequence shown, now player's turn
                showingSequence = false;
                playerIndex = 0;
                
                // Restore normal colors
                tft.fillRect(20, 50, 30, 30, ST7735_RED);
                tft.fillRect(78, 50, 30, 30, ST7735_GREEN);
                tft.fillRect(20, 100, 30, 30, ST7735_BLUE);
                tft.fillRect(78, 100, 30, 30, ST7735_YELLOW);
                
                // Update instruction
                tft.fillRect(0, 20, 128, 20, ST7735_BLACK);
                tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
                tft.setCursor(20, 25);
                tft.print("Your Turn!");
            }
        }
    } else {
        // Player input phase
        int pressedColor = -1;
        
        if (button_pressed(BTN_UP)) {
            pressedColor = 0; // Red
            tft.fillRect(20, 50, 30, 30, ST7735_WHITE);
            delay(200);
            tft.fillRect(20, 50, 30, 30, ST7735_RED);
        }
        else if (button_pressed(BTN_DOWN)) {
            pressedColor = 1; // Green
            tft.fillRect(78, 50, 30, 30, ST7735_WHITE);
            delay(200);
            tft.fillRect(78, 50, 30, 30, ST7735_GREEN);
        }
        else if (button_pressed(BTN_SELECT)) {
            pressedColor = 2; // Blue
            tft.fillRect(20, 100, 30, 30, ST7735_WHITE);
            delay(200);
            tft.fillRect(20, 100, 30, 30, ST7735_BLUE);
        }
        
        if (pressedColor >= 0) {
            // Check if correct
            if (pressedColor == sequence[playerIndex]) {
                playerIndex++;
                
                // Player completed the sequence
                if (playerIndex >= sequenceLength) {
                    memoryScore++;
                    sequenceLength++;
                    
                    if (sequenceLength > 20) {
                        // Win condition
                        tft.fillScreen(ST7735_BLACK);
                        tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
                        tft.setTextSize(2);
                        tft.setCursor(30, 60);
                        tft.print("YOU WIN!");
                        tft.setTextSize(1);
                        tft.setCursor(20, 85);
                        tft.print("Perfect Memory!");
                        
                        // Non-blocking game over - auto return after 3 seconds
                        delay(3000);
                        currentMode = MODE_GAME_MENU;
                        display_game_menu();
                        return;
                    }
                    
                    // Update score
                    tft.fillRect(0, 0, 128, 15, ST7735_BLACK);
                    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
                    tft.setTextSize(1);
                    tft.setCursor(5, 5);
                    tft.print("MEMORY - Score: ");
                    tft.print(memoryScore);
                    
                    // Start next round
                    showingSequence = true;
                    sequenceDisplayIndex = 0;
                    sequenceTimer = millis();
                    
                    tft.fillRect(0, 20, 128, 20, ST7735_BLACK);
                    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
                    tft.setCursor(15, 25);
                    tft.print("Watch sequence!");
                }
            } else {
                // Wrong answer - game over
                tft.fillScreen(ST7735_BLACK);
                tft.setTextColor(ST7735_RED, ST7735_BLACK);
                tft.setTextSize(2);
                tft.setCursor(20, 60);
                tft.print("GAME OVER");
                tft.setTextSize(1);
                tft.setCursor(30, 85);
                tft.print("Score: ");
                tft.print(memoryScore);
                
                // Non-blocking game over - auto return after 3 seconds
                delay(3000);
                currentMode = MODE_GAME_MENU;
                display_game_menu();
                return;
            }
        }
    }
}
