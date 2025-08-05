// AI Assistant mode header for TFT Display project
#ifndef AI_ASSISTANT_MODE_H
#define AI_ASSISTANT_MODE_H

#include <Arduino.h>
#include "audio_recording.h"
#include "deepgram_transcription.h"
#include "gemini_ai.h"

// AI Assistant states
enum AIAssistantState {
    AI_STATE_INACTIVE,
    AI_STATE_WAITING,
    AI_STATE_LISTENING,
    AI_STATE_PROCESSING,
    AI_STATE_AI_THINKING,
    AI_STATE_SHOWING_TRANSCRIPTION,
    AI_STATE_SHOWING_RESPONSE,
    AI_STATE_ERROR
};

// Main AI Assistant functions
void ai_assistant_init();
void ai_assistant_enter();
void ai_assistant_exit();
void ai_assistant_update();
void ai_assistant_connect_wifi();

// Display functions
void ai_assistant_show_welcome();
void ai_assistant_show_main_interface();
void ai_assistant_show_transcription(String text);
void ai_assistant_show_ai_response(String response);
void ai_assistant_show_error(String error);

// State management functions
void ai_assistant_update_waiting();
void ai_assistant_start_recording();
void ai_assistant_update_listening();
void ai_assistant_stop_recording();
void ai_assistant_update_processing();
void ai_assistant_update_ai_thinking();

// Audio processing
void ai_assistant_process_audio();

// Drawing utilities
void ai_assistant_draw_wrapped_text(String text, int start_y, int max_height, uint16_t color);
void ai_assistant_draw_microphone(int x, int y, uint16_t color);
void ai_assistant_draw_ai_icon(int x, int y, uint16_t color);

#endif // AI_ASSISTANT_MODE_H
