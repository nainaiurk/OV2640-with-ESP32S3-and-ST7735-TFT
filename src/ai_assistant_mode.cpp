// AI Assistant mode implementation for TFT Display project
#include "ai_assistant_mode.h"
#include "display.h"
#include "button.h"
#include "menu.h"
#include "audio_recording.h"
#include "deepgram_transcription.h"
#include "gemini_ai.h"
#include <WiFi.h>

// AI Assistant state variables
static AIAssistantState ai_state = AI_STATE_INACTIVE;
static bool was_recording = false;
static unsigned long last_animation_update = 0;
static int animation_frame = 0;
static String current_transcription = "";
static String current_ai_response = "";

const char* WIFI_SSID = "SUST WiFi";           // Update with your WiFi SSID
const char* WIFI_PASSWORD = "SUST10s10";   // Update with your WiFi password
const char* DEEPGRAM_API_KEY = "85e33bfe537cc2e30ac77a649142dc60e7b3b89a";  // Update with your Deepgram API key
const char* GEMINI_API_KEY = "AIzaSyDGLIPiuzORNrIi-qyuW5N26ri9gUYo0DI"; 
const char* AUDIO_FILE = "/voice_recording.wav";  // Audio file path

// Audio recording settings
#define I2S_SCK   17   // Serial Clock (BCLK)

// Audio recording settings

void ai_assistant_init() {
    Serial.println("Initializing AI Assistant...");
    
    // Initialize I2S Recording Services (proven project style)
    I2S_Record_Init();
    
    // Note: Deepgram and Gemini services will be initialized when WiFi is connected
    Serial.println("Audio recording initialized, AI services will be ready when WiFi connects");
    
    ai_state = AI_STATE_INACTIVE;
    Serial.println("AI Assistant initialized successfully");
}

void ai_assistant_enter() {
    Serial.println("Entering AI Assistant mode");
    ai_assistant_show_welcome();
    delay(1000);
    
    // Connect to WiFi
    ai_assistant_connect_wifi();
    
    ai_assistant_show_main_interface();
    ai_state = AI_STATE_WAITING;
}

void ai_assistant_exit() {
    Serial.println("Exiting AI Assistant mode");
    ai_state = AI_STATE_INACTIVE;
    currentMode = MODE_MENU;
    display_menu();
}

void ai_assistant_connect_wifi() {
    Serial.println("Connecting to WiFi...");
    
    // Show WiFi connection screen
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_CYAN);
    tft.setTextSize(1);
    tft.setCursor(5, 20);
    tft.println("Connecting to WiFi...");
    
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(5, 40);
    tft.print("Network: ");
    tft.println(WIFI_SSID);
    
    // Check if already connected
    if (WiFi.status() == WL_CONNECTED) {
        tft.setTextColor(ST7735_GREEN);
        tft.setCursor(5, 60);
        tft.println("Already connected!");
        tft.setCursor(5, 75);
        tft.print("IP: ");
        tft.println(WiFi.localIP());
        delay(1500);
        return;
    }
    
    // Start WiFi connection
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int dots = 0;
    int attempts = 0;
    const int max_attempts = 20; // 10 seconds timeout
    
    while (WiFi.status() != WL_CONNECTED && attempts < max_attempts) {
        delay(500);
        attempts++;
        
        // Show connecting animation
        tft.fillRect(5, 60, 118, 20, ST7735_BLACK);
        tft.setTextColor(ST7735_YELLOW);
        tft.setCursor(5, 60);
        tft.print("Connecting");
        for (int i = 0; i < (dots % 4); i++) {
            tft.print(".");
        }
        dots++;
        
        // Show signal strength if scanning
        if (attempts % 4 == 0) {
            tft.fillRect(5, 80, 118, 10, ST7735_BLACK);
            tft.setCursor(5, 80);
            tft.print("Attempt ");
            tft.print(attempts);
            tft.print("/");
            tft.print(max_attempts);
        }
    }
    
    // Show connection result
    tft.fillRect(5, 60, 118, 40, ST7735_BLACK);
    
    if (WiFi.status() == WL_CONNECTED) {
        tft.setTextColor(ST7735_GREEN);
        tft.setCursor(5, 60);
        tft.println("Connected!");
        tft.setCursor(5, 75);
        tft.print("IP: ");
        tft.println(WiFi.localIP());
        tft.setCursor(5, 90);
        tft.print("Signal: ");
        tft.print(WiFi.RSSI());
        tft.println(" dBm");
        
        Serial.println("WiFi connected successfully");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        
        delay(2000);
    } else {
        tft.setTextColor(ST7735_RED);
        tft.setCursor(5, 60);
        tft.println("Connection failed!");
        tft.setCursor(5, 75);
        tft.println("Check credentials");
        tft.setCursor(5, 90);
        tft.println("or network signal");
        
        Serial.println("WiFi connection failed");
        Serial.println("Please check WiFi credentials and network availability");
        
        delay(3000);
    }
}

void ai_assistant_update() {
    // Handle back button
    if (button_pressed(BTN_BACK)) {
        ai_assistant_exit();
        return;
    }
    
    // Handle recording button (using SELECT button)
    bool recording_button = button_pressed(BTN_SELECT);
    
    switch (ai_state) {
        case AI_STATE_WAITING:
            ai_assistant_update_waiting();
            if (recording_button && !was_recording) {
                ai_assistant_start_recording();
            }
            break;
            
        case AI_STATE_LISTENING:
            ai_assistant_update_listening();
            if (!recording_button && was_recording) {
                ai_assistant_stop_recording();
            }
            break;
            
        case AI_STATE_PROCESSING:
            ai_assistant_update_processing();
            break;
            
        case AI_STATE_AI_THINKING:
            ai_assistant_update_ai_thinking();
            break;
            
        case AI_STATE_SHOWING_TRANSCRIPTION:
        case AI_STATE_SHOWING_RESPONSE:
            // Auto return to waiting after showing results
            if (millis() - last_animation_update > 8000) {
                ai_assistant_show_main_interface();
                ai_state = AI_STATE_WAITING;
            }
            break;
            
        case AI_STATE_ERROR:
            if (millis() - last_animation_update > 3000) {
                ai_assistant_show_main_interface();
                ai_state = AI_STATE_WAITING;
            }
            break;
    }
    
    was_recording = (ai_state == AI_STATE_LISTENING);
}

void ai_assistant_show_welcome() {
    tft.fillScreen(ST7735_BLACK);
    
    // Draw gradient background
    for(int y = 0; y < 160; y++) {
        uint16_t color = tft.color565(10 + y/12, 5 + y/20, 30 + y/6);
        tft.drawFastHLine(0, y, 128, color);
    }
    
    // Title
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.setCursor(35, 25);
    tft.println("VOICE");
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(15, 50);
    tft.println("ASSISTANT");
    
    // AI icon
    ai_assistant_draw_ai_icon(128/2 - 15, 80, ST7735_CYAN);
    
    // Loading bar
    tft.drawRoundRect(15, 140, 128 - 30, 10, 5, ST7735_WHITE);
    for(int i = 0; i < 128 - 34; i += 2) {
        tft.fillRoundRect(17, 142, i, 6, 3, ST7735_MAGENTA);
        delay(20);
    }
}

void ai_assistant_show_main_interface() {
    tft.fillScreen(ST7735_BLACK);
    
    // Title
    tft.setTextColor(ST7735_MAGENTA);
    tft.setTextSize(2);
    tft.setCursor(27, 30);
    tft.println("ASK ME");
    tft.setTextColor(ST7735_CYAN);
    tft.setCursor(15, 55);
    tft.println("ANYTHING");
    
    // Microphone icon
    ai_assistant_draw_microphone(128/2 - 15, 85, ST7735_WHITE);
    
    // Instructions
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(30, 135);
    tft.println("Hold To Speak");
    
    // WiFi status indicator
    tft.setTextSize(1);
    if (WiFi.status() == WL_CONNECTED) {
        tft.setTextColor(ST7735_GREEN);
        tft.setCursor(5, 150);
        tft.print("WiFi: Connected");
        // WiFi signal strength icon
        int rssi = WiFi.RSSI();
        int bars = 0;
        if (rssi > -30) bars = 4;
        else if (rssi > -50) bars = 3;
        else if (rssi > -70) bars = 2;
        else if (rssi > -80) bars = 1;
        
        for(int i = 0; i < 4; i++) {
            uint16_t color = (i < bars) ? ST7735_GREEN : ST7735_RED;
            tft.fillRect(105 + i*3, 152 - i*2, 2, 2 + i*2, color);
        }
    } else {
        tft.setTextColor(ST7735_RED);
        tft.setCursor(5, 150);
        tft.print("WiFi: Disconnected");
    }
    
    last_animation_update = millis();
    animation_frame = 0;
}

void ai_assistant_update_waiting() {
    if (millis() - last_animation_update > 80) {
        // Pulsing microphone animation
        int pulse = 128 + 127 * sin(animation_frame * 0.1);
        uint16_t ring_color = tft.color565(pulse/4, pulse/4, pulse/2);
        
        // Clear previous ring
        tft.drawCircle(128/2, 100, 25, ST7735_BLACK);
        tft.drawCircle(128/2, 100, 26, ST7735_BLACK);
        
        // Draw new pulsing ring
        tft.drawCircle(128/2, 100, 25, ring_color);
        
        animation_frame++;
        last_animation_update = millis();
    }
}

void ai_assistant_start_recording() {
    Serial.println("🎙️ Recording started...");
    
    ai_state = AI_STATE_LISTENING;
    was_recording = true;  // Set recording flag
    
    // Show listening interface
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.println("LISTENING...");
    tft.drawFastHLine(5, 18, 128-10, ST7735_RED);
    
    ai_assistant_draw_microphone(128/2 - 15, 40, ST7735_RED);
    
    animation_frame = 0;
    last_animation_update = millis();
    
    delay(30);  // unbouncing & suppressing button 'click' noise like proven project
    
    // Start recording once (proven project pattern)
    Record_Start(AUDIO_FILE);
}

void ai_assistant_update_listening() {
    // Continue recording like proven project (keep calling Record_Start to continue recording)
    Record_Start(AUDIO_FILE);
    
    if (millis() - last_animation_update > 50) {
        // Clear waveform area
        tft.fillRect(0, 90, 128, 50, ST7735_BLACK);
        
        // Draw animated waveform
        for(int x = 5; x < 123; x += 3) {
            int height = 5 + 15 * sin((x + animation_frame * 4) * 0.2) * cos((x + animation_frame * 2) * 0.1);
            int y = 115 - height/2;
            uint16_t wave_color = tft.color565(255, 100 - height*3, 100 - height*3);
            tft.drawLine(x, y, x, y + height, wave_color);
        }
        
        // Recording indicator
        int pulse = 128 + 127 * sin(animation_frame * 0.2);
        tft.fillCircle(15, 25, 3, tft.color565(pulse, 0, 0));
        
        animation_frame++;
        last_animation_update = millis();
    }
}

void ai_assistant_stop_recording() {
    Serial.println("🛑 Recording stopped");
    was_recording = false;  // Reset recording flag
    
    float recorded_seconds;
    if (Record_Available(AUDIO_FILE, &recorded_seconds)) {  // Proven project pattern
        if (recorded_seconds > 0.4) {  // Like proven project
            Serial.printf("Recording duration: %.1fs\n", recorded_seconds);
            ai_assistant_process_audio();
        } else {
            Serial.println("⚠️ Recording too short (< 0.4s)");
            ai_assistant_show_error("Recording too short. Hold button longer while speaking.");
            delay(2000);
            ai_assistant_show_main_interface();
            ai_state = AI_STATE_WAITING;
        }
    } else {
        ai_assistant_show_error("Recording failed");
        delay(2000);
        ai_assistant_show_main_interface();
        ai_state = AI_STATE_WAITING;
    }
}

void ai_assistant_update_processing() {
    if (millis() - last_animation_update > 80) {
        // Clear animation area
        tft.fillRect(0, 90, 128, 50, ST7735_BLACK);
        
        // Rotating dots animation
        int center_x = 128/2;
        int center_y = 60;
        int radius = 30;
        
        for(int i = 0; i < 8; i++) {
            float angle = (animation_frame * 0.3 + i * 45) * PI / 180;
            int x = center_x + radius * cos(angle);
            int y = center_y + radius * sin(angle);
            
            int brightness = 255 - i * 25;
            uint16_t dot_color = tft.color565(brightness/2, brightness/3, brightness);
            tft.fillCircle(x, y, 2, dot_color);
        }
        
        animation_frame++;
        last_animation_update = millis();
    }
}

void ai_assistant_update_ai_thinking() {
    if (millis() - last_animation_update > 100) {
        // Clear animation area
        tft.fillRect(0, 90, 128, 50, ST7735_BLACK);
        
        // Neural network animation
        int center_x = 128/2;
        int center_y = 60;
        
        for(int i = 0; i < 6; i++) {
            float angle = (animation_frame * 0.15 + i * 60) * PI / 180;
            int brightness = 100 + 155 * sin(animation_frame * 0.2 + i * 1.0);
            uint16_t color = tft.color565(brightness/3, brightness/5, brightness);
            
            int x1 = center_x + 25 * cos(angle);
            int y1 = center_y + 25 * sin(angle);
            int x2 = center_x + 35 * cos(angle);
            int y2 = center_y + 35 * sin(angle);
            
            tft.drawLine(x1, y1, x2, y2, color);
            tft.fillCircle(x2, y2, 2, color);
        }
        
        animation_frame++;
        last_animation_update = millis();
    }
}

void ai_assistant_process_audio() {
    // Check WiFi connection first
    if (WiFi.status() != WL_CONNECTED) {
        ai_assistant_show_error("WiFi not connected. Please check your connection.");
        return;
    }
    
    // Re-initialize AI processor if not ready (this handles the case where WiFi connected after init)
    // Note: Direct function calls don't require initialization checks
    Serial.println("Processing audio with Deepgram...");
    
    // Simulate audio processing and transcription
    Serial.println("🔄 Processing audio...");
    current_transcription = SpeechToText_Deepgram(AUDIO_FILE);
    
    if (current_transcription.length() == 0 || current_transcription.startsWith("Error")) {
        ai_assistant_show_error("Speech recognition failed. Try speaking louder or closer to microphone.");
        return;
    }
    
    // Show transcription
    ai_assistant_show_transcription(current_transcription);
    delay(3000);
    
    // Process with AI
    Serial.println("🤖 Processing with AI...");
    ai_state = AI_STATE_AI_THINKING;
    tft.fillRect(0, 0, 128, 25, ST7735_BLACK);
    tft.setTextColor(ST7735_MAGENTA);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.println("AI THINKING...");
    tft.drawFastHLine(5, 18, 128-10, ST7735_MAGENTA);
    
    animation_frame = 0;
    last_animation_update = millis();
    
    // Get AI response
    current_ai_response = Gemini_ProcessText(current_transcription);
    
    if (current_ai_response.startsWith("Error")) {
        ai_assistant_show_error(current_ai_response);
        return;
    }
    
    // Show AI response
    ai_assistant_show_ai_response(current_ai_response);
}

void ai_assistant_show_transcription(String text) {
    ai_state = AI_STATE_SHOWING_TRANSCRIPTION;
    
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_GREEN);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.println("YOUR SPEECH:");
    tft.drawFastHLine(5, 18, 128-10, ST7735_GREEN);
    
    // Display text with word wrapping
    ai_assistant_draw_wrapped_text(text, 25, 100, ST7735_WHITE);
    
    last_animation_update = millis();
}

void ai_assistant_show_ai_response(String response) {
    ai_state = AI_STATE_SHOWING_RESPONSE;
    
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_CYAN);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.println("AI RESPONSE:");
    tft.drawFastHLine(5, 18, 128-10, ST7735_CYAN);
    
    // Display response with word wrapping
    ai_assistant_draw_wrapped_text(response, 25, 100, ST7735_CYAN);
    
    last_animation_update = millis();
}

void ai_assistant_show_error(String error) {
    ai_state = AI_STATE_ERROR;
    
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(1);
    tft.setCursor(5, 5);
    tft.println("ERROR:");
    tft.drawFastHLine(5, 18, 128-10, ST7735_RED);
    
    ai_assistant_draw_wrapped_text(error, 25, 100, ST7735_YELLOW);
    
    last_animation_update = millis();
}

void ai_assistant_draw_wrapped_text(String text, int start_y, int max_height, uint16_t color) {
    tft.setTextColor(color);
    tft.setTextSize(1);
    
    int y = start_y;
    int x = 5;
    int line_height = 9;
    int char_width = 6;
    int max_chars = (128 - 10) / char_width;
    
    for(int i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if(c == ' ' && (i % max_chars) > (max_chars - 5)) {
            y += line_height;
            x = 5;
            if(y > start_y + max_height - 15) break;
            continue;
        }
        
        if((i % max_chars) == 0 && i > 0) {
            y += line_height;
            x = 5;
            if(y > start_y + max_height - 15) break;
        }
        
        tft.setCursor(x, y);
        tft.print(c);
        x += char_width;
    }
}

void ai_assistant_draw_microphone(int x, int y, uint16_t color) {
    // Main microphone body
    tft.fillRoundRect(x + 10, y + 5, 10, 15, 5, color);
    
    // Grille lines
    for(int i = 0; i < 3; i++) {
        tft.drawFastHLine(x + 12, y + 8 + i * 3, 6, ST7735_BLACK);
    }
    
    // Stand and base
    tft.drawRoundRect(x + 5, y + 20, 20, 8, 4, color);
    tft.drawLine(x + 15, y + 28, x + 15, y + 32, color);
    tft.drawFastHLine(x + 10, y + 32, 10, color);
}

void ai_assistant_draw_ai_icon(int x, int y, uint16_t color) {
    // AI brain outline
    tft.drawCircle(x + 15, y + 12, 12, color);
    
    // Brain hemispheres
    tft.drawLine(x + 15, y + 2, x + 15, y + 22, color);
    
    // Neural pathways
    tft.drawLine(x + 8, y + 8, x + 22, y + 16, color);
    tft.drawLine(x + 8, y + 16, x + 22, y + 8, color);
    
    // Synapses
    tft.fillCircle(x + 8, y + 8, 1, color);
    tft.fillCircle(x + 22, y + 8, 1, color);
    tft.fillCircle(x + 8, y + 16, 1, color);
    tft.fillCircle(x + 22, y + 16, 1, color);
    tft.fillCircle(x + 15, y + 12, 1, color);
}
