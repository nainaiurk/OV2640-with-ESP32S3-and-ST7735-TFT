#include "camera.h"
#include "camera_pin.h"
#include "sd_card.h"
#include <Arduino.h>

// Global variable to store last captured frame
static camera_fb_t* last_captured_frame = nullptr;
static uint8_t* captured_frame_buffer = nullptr;
static size_t captured_frame_size = 0;

bool camera_init() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    // Higher resolution settings for better image quality
    config.xclk_freq_hz = 24000000; // Increased for better performance
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA; // Upgraded to 320x240
    config.jpeg_quality = 8; // Better quality (lower = better)
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    
    if(psramFound()){
        config.fb_count = 2;
    }

    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return false;
    }

    return true;
}

void camera_configure() {
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_hmirror(s, 1);
        s->set_vflip(s, 0);
        s->set_framesize(s, FRAMESIZE_QVGA); // Match the init setting
        s->set_quality(s, 8); // Better quality
        
        // Enhanced brightness and exposure settings
        s->set_brightness(s, 2); // Maximum brightness
        s->set_contrast(s, 2); // Maximum contrast  
        s->set_saturation(s, 1); // Slight saturation boost
        
        // Auto exposure and gain control for brightness
        s->set_exposure_ctrl(s, 1); // Enable auto exposure
        s->set_gain_ctrl(s, 1); // Enable auto gain
        s->set_awb_gain(s, 1); // Auto white balance gain
        s->set_wb_mode(s, 0); // Auto white balance
        
        // Advanced settings for maximum brightness and quality
        s->set_aec2(s, 1); // Enable AEC sensor
        s->set_ae_level(s, 2); // Maximum auto exposure level
        s->set_agc_gain(s, 20); // Even higher gain for brightness
        s->set_aec_value(s, 800); // Manual exposure value for brightness
        s->set_lenc(s, 1); // Enable lens correction
        s->set_bpc(s, 1); // Black pixel correction
        s->set_wpc(s, 1); // White pixel correction
        
        // Additional brightness tweaks
        s->set_dcw(s, 1); // Enable downsample
        s->set_raw_gma(s, 1); // Enable gamma correction
        
        Serial.println("Camera configured: QVGA 320x240 with maximum brightness");
    }
}

camera_fb_t* camera_get_frame() {
    return esp_camera_fb_get();
}

void camera_return_frame(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

bool camera_capture_and_save() {
    // Get a frame for saving
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return false;
    }
    
    // Free previous capture if exists
    if (captured_frame_buffer) {
        free(captured_frame_buffer);
        captured_frame_buffer = nullptr;
    }
    
    // Allocate memory for the captured frame
    captured_frame_size = fb->len;
    captured_frame_buffer = (uint8_t*)malloc(captured_frame_size);
    
    if (!captured_frame_buffer) {
        Serial.println("Failed to allocate memory for captured frame");
        esp_camera_fb_return(fb);
        return false;
    }
    
    // Copy the frame data
    memcpy(captured_frame_buffer, fb->buf, captured_frame_size);
    
    // Create a static frame buffer structure for the captured frame
    static camera_fb_t captured_fb;
    captured_fb.buf = captured_frame_buffer;
    captured_fb.len = captured_frame_size;
    captured_fb.width = fb->width;
    captured_fb.height = fb->height;
    captured_fb.format = fb->format;
    captured_fb.timestamp = fb->timestamp;
    
    last_captured_frame = &captured_fb;
    
    Serial.printf("Image captured! Size: %dx%d, JPEG bytes: %d\n", 
                  fb->width, fb->height, fb->len);
    
    // Try to save to SD card (non-blocking, continue even if it fails)
    if (sd_card_is_mounted()) {
        if (save_photo_to_sd(captured_frame_buffer, captured_frame_size)) {
            Serial.println("Photo saved to SD card successfully!");
        } else {
            Serial.println("Failed to save photo to SD card");
        }
    } else {
        Serial.println("SD card not available - photo saved in memory only");
    }
    
    // Return the original frame buffer
    esp_camera_fb_return(fb);
    
    Serial.println("Image capture complete!");
    return true;
}

camera_fb_t* camera_get_last_capture() {
    return last_captured_frame;
}

void camera_release_last_capture() {
    if (captured_frame_buffer) {
        free(captured_frame_buffer);
        captured_frame_buffer = nullptr;
        captured_frame_size = 0;
        last_captured_frame = nullptr;
    }
}
