#pragma once

#include <Arduino.h>
#include "app_controller.h"

// Face recognition menu options
enum FaceRecognitionMenuOption {
    FACE_MENU_ENROLL = 0,
    FACE_MENU_RECOGNIZE = 1,
    FACE_MENU_VIEW_DATABASE = 2,
    FACE_MENU_DELETE_ALL = 3,
    FACE_MENU_BACK = 4
};

// Function declarations
bool face_recognition_init();
void cleanup_face_recognition();

// Menu functions
void display_face_recognition_menu();
FaceRecognitionMenuOption handle_face_recognition_menu_input();
void display_face_recognition_screen(AppMode mode);

// Face operations
void handle_face_enrollment();
void handle_face_recognition();

// Database management
void show_face_database_info();
bool confirm_delete_all_faces();
void delete_all_faces();
