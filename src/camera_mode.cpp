#include "camera_mode.h"
#include "display.h"
#include "camera.h"
#include "menu.h"
#include "button.h"
#include <TJpg_Decoder.h>

extern bool cameraAvailable;

void handle_camera_mode() {
    // Check if camera is available
    if (!cameraAvailable) {
        show_camera_error();
        return;
    }
    
    static uint32_t frame_count = 0;
    static uint32_t last_time = 0;
    
    // Check for back button to return to menu
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to menu...");
        currentMode = MODE_MENU;
        display_menu();
        return;
    }
    
    // Get and display camera frame
    camera_fb_t *fb = camera_get_frame();
    if (fb) {
        if (fb->len > 0) {
            TJpgDec.drawJpg(0, 0, fb->buf, fb->len);
            frame_count++;
            
            // Debug frame info every 100 frames
            if (frame_count % 100 == 0) {
                Serial.printf("Frame size: %dx%d, JPEG bytes: %d\n", fb->width, fb->height, fb->len);
            }
        }
        
        // Return frame buffer
        camera_return_frame(fb);
        
        // Show FPS every 3 seconds
        uint32_t current_time = millis();
        if (current_time - last_time >= 3000) {
            float fps = (float)frame_count / ((current_time - last_time) / 1000.0);
            Serial.printf("FPS: %.1f | Resolution: QVGA (320x240) -> Display (128x160)\n", fps);
            frame_count = 0;
            last_time = current_time;
        }
    }
}

void show_camera_error() {
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 60);
    tft.print("Camera Not Available!");
    
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to menu from camera error...");
        currentMode = MODE_MENU;
        display_menu();
    }
    delay(100);
}
