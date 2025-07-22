#include "display.h"

// Initialize TFT object
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void display_init() {
    Serial.println("Initializing 1.8\" ST7735 display...");
    
    // For 1.8" ST7735 displays, use proper initialization
    // Try BLACKTAB first (common for 1.8" displays)
    tft.initR(INITR_BLACKTAB);
    
    // Set rotation to 180 degrees (upside down)
    tft.setRotation(2); // 180 degree rotation
    
    // Get display dimensions
    int width = tft.width();
    int height = tft.height();
    Serial.printf("Display dimensions: %dx%d\n", width, height);
    
    // For 1.8" displays, ensure we're using full 128x160 resolution
    if (width != 128 || height != 160) {
        Serial.println("Trying alternative initialization for 1.8\" display...");
        tft.initR(INITR_GREENTAB);
        tft.setRotation(2); // Keep 180 degree rotation
        width = tft.width();
        height = tft.height();
        Serial.printf("New dimensions: %dx%d\n", width, height);
    }
    
    // Clear entire display with black
    tft.fillScreen(ST7735_BLACK);
    
    // Additional clearing method for stubborn displays
    tft.fillRect(0, 0, width, height, ST7735_BLACK);
    
    // Force manual clearing if needed
    tft.startWrite();
    tft.setAddrWindow(0, 0, width, height);
    for (int i = 0; i < width * height; i++) {
        tft.pushColor(ST7735_BLACK);
    }
    tft.endWrite();
    
    // Set default text properties
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setTextWrap(true);
    
    Serial.println("1.8\" ST7735 display initialized with full black coverage");
}

// JPEG decoder callback function - QVGA (320x240) to ST7735 (128x140) with clean scaling
// Limited to upper 140 pixels to leave space for buttons
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data) {
    // Safety checks
    if (!data || w == 0 || h == 0) return true;
    
    // Check if we're drawing a thumbnail (from photos_mode)
    extern bool is_drawing_thumbnail;
    extern int thumbnail_x, thumbnail_y, thumbnail_w, thumbnail_h;
    
    if (is_drawing_thumbnail) {
        // For thumbnails, we need to map the decoded JPEG coordinates to thumbnail position
        // The JPEG decoder gives us coordinates relative to the original image
        // We need to scale and offset them to fit in our thumbnail area
        
        int16_t dest_x = thumbnail_x + x;
        int16_t dest_y = thumbnail_y + y;
        uint16_t dest_w = w;
        uint16_t dest_h = h;
        
        // Clip to thumbnail boundaries
        if (dest_x < thumbnail_x) {
            int clip = thumbnail_x - dest_x;
            dest_w -= clip;
            data += clip; // Skip clipped pixels
            dest_x = thumbnail_x;
        }
        if (dest_y < thumbnail_y) {
            int clip = thumbnail_y - dest_y;
            dest_h -= clip;
            data += clip * w; // Skip clipped rows
            dest_y = thumbnail_y;
        }
        if (dest_x + dest_w > thumbnail_x + thumbnail_w) {
            dest_w = thumbnail_x + thumbnail_w - dest_x;
        }
        if (dest_y + dest_h > thumbnail_y + thumbnail_h) {
            dest_h = thumbnail_y + thumbnail_h - dest_y;
        }
        
        // Only draw if there's something to draw
        if (dest_w > 0 && dest_h > 0) {
            tft.startWrite();
            tft.setAddrWindow(dest_x, dest_y, dest_w, dest_h);
            
            // Copy pixels directly from the decoded JPEG data
            for (int row = 0; row < dest_h; row++) {
                for (int col = 0; col < dest_w; col++) {
                    if ((row * w + col) < (w * h)) {
                        tft.pushColor(data[row * w + col]);
                    }
                }
            }
            tft.endWrite();
        }
        return true;
    }
    
    // Original camera display logic
    // Use integer scaling to avoid grid artifacts
    // Scale down by taking every 2nd or 3rd pixel consistently
    int16_t display_x = x >> 1; // Divide by 2 (bit shift)
    int16_t display_y = y >> 1; // Divide by 2 (bit shift)
    
    // Only draw if within display bounds and above button area (y < 140)
    if (display_x >= 128 || display_y >= 140) {
        return true;
    }
    
    // Calculate scaled dimensions using integer division
    uint16_t display_w = w >> 1; // Divide by 2
    uint16_t display_h = h >> 1; // Divide by 2
    
    // Bounds check - limit to camera display area (140 pixels height)
    if (display_x + display_w > 128) display_w = 128 - display_x;
    if (display_y + display_h > 140) display_h = 140 - display_y;
    
    if (display_w == 0 || display_h == 0) return true;
    
    // Clean pixel transfer with brightness boosting
    tft.startWrite();
    tft.setAddrWindow(display_x, display_y, display_w, display_h);
    
    // Sample pixels with consistent 2x2 sampling to avoid grid
    for (uint16_t j = 0; j < display_h; j++) {
        uint16_t src_y = j << 1; // Multiply by 2
        if (src_y >= h) break;
        
        for (uint16_t i = 0; i < display_w; i++) {
            uint16_t src_x = i << 1; // Multiply by 2
            if (src_x >= w) break;
            
            uint16_t src_index = src_y * w + src_x;
            if (src_index < w * h) {
                uint16_t pixel = data[src_index];
                
                // Extract RGB components
                uint16_t r = (pixel >> 11) & 0x1F;
                uint16_t g = (pixel >> 5) & 0x3F;
                uint16_t b = pixel & 0x1F;
                
                // Moderate brightness boost to avoid oversaturation
                r = (r * 3) >> 1; // 1.5x multiplier
                g = (g * 3) >> 1; // 1.5x multiplier
                b = (b * 3) >> 1; // 1.5x multiplier
                
                // Clamp to maximum values
                if (r > 31) r = 31;
                if (g > 63) g = 63;
                if (b > 31) b = 31;
                
                // Reconstruct enhanced pixel
                uint16_t enhanced_pixel = (r << 11) | (g << 5) | b;
                tft.pushColor(enhanced_pixel);
            }
        }
    }
    
    tft.endWrite();
    return true;
}
