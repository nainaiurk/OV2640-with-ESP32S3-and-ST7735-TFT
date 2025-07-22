#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// TFT pin definitions
#define TFT_CS   10  // Chip Select
#define TFT_RST  9   // Reset
#define TFT_DC   3   // Data/Command
#define TFT_MOSI 11  // DIN (Data In)
#define TFT_SCLK 12  // CLK (Clock)

// External TFT object
extern Adafruit_ST7735 tft;

// Function declarations
void display_init();
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data);

#endif
