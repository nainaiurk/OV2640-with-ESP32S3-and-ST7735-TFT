#ifndef CAMERA_H
#define CAMERA_H

#include <esp_camera.h>

// Function declarations
bool camera_init();
void camera_configure();
camera_fb_t* camera_get_frame();
void camera_return_frame(camera_fb_t* fb);
bool camera_capture_and_save();
camera_fb_t* camera_get_last_capture();
void camera_release_last_capture();

#endif
