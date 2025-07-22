#ifndef PHOTOS_MODE_H
#define PHOTOS_MODE_H

#include <Arduino.h>
#include "sd_card.h"
#include "display.h"
#include "button.h"

// Photos mode states
enum PhotosState {
    PHOTOS_LIST,      // Showing list of photos
    PHOTOS_VIEWING,   // Viewing a single photo
    PHOTOS_CONFIRM_DELETE // Confirmation dialog for deletion
};

// Photos mode configuration
#define PHOTOS_PER_PAGE 6
#define PHOTO_LIST_START_Y 30
#define GRID_COLS 3
#define GRID_ROWS 2
#define THUMBNAIL_WIDTH 40
#define THUMBNAIL_HEIGHT 30
#define THUMBNAIL_SPACING 2

// Thumbnail drawing variables
extern int thumbnail_x, thumbnail_y, thumbnail_w, thumbnail_h;
extern bool is_drawing_thumbnail;

// Function declarations
void photos_mode_init();
void handle_photos_mode();
void display_photos_list();
void display_photo_viewer();
void display_delete_confirmation();

// Navigation functions
void photos_scroll_up();
void photos_scroll_down();
void photos_select_current();
void photos_go_back();
void photos_delete_current();

// Helper functions
void draw_photo_list_item(int index, int y_pos, bool selected);
void draw_photo_grid(); // New grid display function
void draw_thumbnail(int grid_index, int photo_index, bool selected);
void load_and_display_photo(const char* filename);
void show_photo_loading();
void show_no_photos_message();
void format_file_size(long size, char* buffer, int buffer_size);

#endif
