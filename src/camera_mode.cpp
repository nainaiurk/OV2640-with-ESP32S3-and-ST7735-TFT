#include "camera_mode.h"
#include "display.h"
#include "camera.h"
#include "menu.h"
#include "button.h"
#include "sd_card.h"
#include <TJpg_Decoder.h>

extern bool cameraAvailable;

// Camera display area dimensions (leaving space for buttons at bottom)
#define CAMERA_DISPLAY_HEIGHT 140  // Reserve 20 pixels at bottom for buttons
#define BUTTON_AREA_HEIGHT 20
#define BUTTON_Y_START 140

// Camera mode states
enum CameraState {
    CAMERA_LIVE_VIEW,
    CAMERA_SHOWING_CAPTURE
};

static CameraState camera_state = CAMERA_LIVE_VIEW;

void draw_camera_buttons() {
    // Clear button area
    tft.fillRect(0, BUTTON_Y_START, 128, BUTTON_AREA_HEIGHT, ST7735_BLACK);
    
    // Draw button backgrounds
    tft.fillRect(5, BUTTON_Y_START + 2, 50, 16, ST7735_BLUE);      // CAPTURE button
    tft.fillRect(73, BUTTON_Y_START + 2, 50, 16, ST7735_RED);      // BACK button
    
    // Draw button text
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    
    // CAPTURE button text
    tft.setCursor(8, BUTTON_Y_START + 6);
    tft.print("CAPTURE");
    
    // BACK button text  
    tft.setCursor(85, BUTTON_Y_START + 6);
    tft.print("BACK");
}

void show_captured_image() {
    camera_fb_t* captured_frame = camera_get_last_capture();
    if (captured_frame && captured_frame->len > 0) {
        // Clear the camera area
        tft.fillRect(0, 0, 128, CAMERA_DISPLAY_HEIGHT, ST7735_BLACK);
        
        // Display the captured image
        TJpgDec.drawJpg(0, 0, captured_frame->buf, captured_frame->len);
        
        // Add a border to indicate it's a captured image
        tft.drawRect(0, 0, 128, CAMERA_DISPLAY_HEIGHT, ST7735_WHITE);
        tft.drawRect(1, 1, 126, CAMERA_DISPLAY_HEIGHT-2, ST7735_WHITE);
        
        // Update button text to show "CONTINUE"
        tft.fillRect(5, BUTTON_Y_START + 2, 50, 16, ST7735_GREEN);
        tft.setTextColor(ST7735_BLACK);
        tft.setCursor(8, BUTTON_Y_START + 6);
        tft.print("CONTINUE");
        
        Serial.println("Showing captured image");
    }
}

void handle_camera_mode() {
    // Check if camera is available
    if (!cameraAvailable) {
        show_camera_error();
        return;
    }
    
    static uint32_t frame_count = 0;
    static uint32_t last_time = 0;
    static bool buttons_drawn = false;
    static bool up_button_was_pressed = false;
    static bool back_button_was_pressed = false;
    
    // Check current button states
    bool up_button_current = button_pressed(BTN_UP);
    bool back_button_current = button_pressed(BTN_BACK);
    
    // Check for back button to return to menu (edge detection)
    if (back_button_current && !back_button_was_pressed) {
        Serial.println("BACK button pressed - returning to menu...");
        currentMode = MODE_MENU;
        camera_release_last_capture(); // Clean up captured frame
        camera_state = CAMERA_LIVE_VIEW; // Reset state
        display_menu();
        buttons_drawn = false; // Reset for next time
        return;
    }
    
    // Handle different camera states
    switch (camera_state) {
        case CAMERA_LIVE_VIEW:
        {
            // Draw buttons once
            if (!buttons_drawn) {
                draw_camera_buttons();
                buttons_drawn = true;
            }
            
            // Check for capture button (UP button) - edge detection
            if (up_button_current && !up_button_was_pressed) {
                Serial.println("UP button pressed - capturing image...");
                
                // Flash the capture button to show it's pressed
                tft.fillRect(5, BUTTON_Y_START + 2, 50, 16, ST7735_YELLOW);
                tft.setTextColor(ST7735_BLACK);
                tft.setCursor(8, BUTTON_Y_START + 6);
                tft.print("CAPTURE");
                
                // Perform capture
                if (camera_capture_and_save()) {
                    camera_state = CAMERA_SHOWING_CAPTURE;
                    show_captured_image();
                    Serial.println("Stopping camera feed - showing captured image");
                } else {
                    // Restore button appearance if capture failed
                    draw_camera_buttons();
                }
            } else {
                // ONLY process camera feed in LIVE_VIEW state and when not capturing
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
                        Serial.printf("FPS: %.1f | Resolution: QVGA (320x240) -> Display (128x%d)\n", fps, CAMERA_DISPLAY_HEIGHT);
                        frame_count = 0;
                        last_time = current_time;
                    }
                }
                
                // Redraw buttons to ensure they stay visible
                if (frame_count % 50 == 0) {
                    draw_camera_buttons();
                }
            }
            break;
        }
            
        case CAMERA_SHOWING_CAPTURE:
        {
            // CRITICAL: NO camera frame processing in this state!
            // The captured image should remain static on screen
            
            // Check ONLY for UP button to continue capturing (NO automatic timeout)
            if (up_button_current && !up_button_was_pressed) {
                Serial.println("UP button pressed - resuming camera feed for next capture...");
                
                camera_state = CAMERA_LIVE_VIEW;
                draw_camera_buttons();
                buttons_drawn = true;
                Serial.println("Camera feed resumed");
            }
            // Important: NO camera_get_frame() or TJpgDec.drawJpg() calls here!
            // Image stays displayed until user presses UP button
            break;
        }
    }
    
    // Update button states for next iteration
    up_button_was_pressed = up_button_current;
    back_button_was_pressed = back_button_current;
}

void show_camera_error() {
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
    tft.setTextSize(1);
    
    // Center the error message
    String errorText = "Camera Not Available!";
    int errorWidth = errorText.length() * 6; // Each character is 6 pixels wide in size 1
    int errorX = (tft.width() - errorWidth) / 2;
    tft.setCursor(errorX, 60);
    tft.print(errorText);
    
    // Draw back button even in error state
    tft.fillRect(73, BUTTON_Y_START + 2, 50, 16, ST7735_RED);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(85, BUTTON_Y_START + 6);
    tft.print("BACK");
    
    if (button_pressed(BTN_BACK)) {
        Serial.println("Returning to menu from camera error...");
        currentMode = MODE_MENU;
        display_menu();
    }
    delay(100);
}
