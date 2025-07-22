#include "photos_mode.h"
#include "menu.h"
#include <TJpg_Decoder.h>

// Global variables for photos mode
static PhotosState photos_state = PHOTOS_LIST;
static PhotoInfo photos_list[MAX_PHOTOS_LIST];
static int photos_count = 0;
static int selected_photo = 0;
static int photos_scroll_offset = 0;
static uint8_t* current_photo_buffer = nullptr;
static size_t current_photo_size = 0;

// Thumbnail drawing variables
int thumbnail_x = 0, thumbnail_y = 0, thumbnail_w = 0, thumbnail_h = 0;
bool is_drawing_thumbnail = false;

// Button state tracking
static bool up_button_was_pressed = false;
static bool down_button_was_pressed = false;
static bool select_button_was_pressed = false;
static bool back_button_was_pressed = false;

void photos_mode_init() {
    Serial.println("Initializing Photos mode...");
    
    // Check SD card status first
    Serial.printf("SD card mounted: %s\n", sd_card_is_mounted() ? "YES" : "NO");
    sd_card_debug_info(); // Show detailed SD card status
    
    // Try to initialize SD card if not already done (but don't block if it fails)
    if (!sd_card_is_mounted()) {
        Serial.println("SD card not mounted, trying to initialize...");
        sd_card_test_pins(); // Test pins before init
        
        if (!sd_card_init()) {
            Serial.println("SD card init failed, running full diagnostics...");
            sd_card_test_connection();
            sd_card_debug_info();
            Serial.println("Continuing without SD card...");
        } else {
            Serial.println("SD card successfully initialized!");
            sd_card_debug_info();
        }
    }
    
    // Set initial state
    photos_state = PHOTOS_LIST;
    selected_photo = 0;
    photos_scroll_offset = 0;
    photos_count = 0;
    
    // Get photos from SD card (if available)
    if (sd_card_is_mounted()) {
        Serial.println("Getting photos list...");
        get_photo_list(photos_list, MAX_PHOTOS_LIST, &photos_count);
        Serial.printf("Found %d photos\n", photos_count);
        
        // Debug: show first few photos
        for (int i = 0; i < photos_count && i < 3; i++) {
            Serial.printf("  Photo %d: %s (%zu bytes)\n", i, photos_list[i].filename, photos_list[i].size);
        }
    } else {
        Serial.println("SD card not available, no photos to display");
        photos_count = 0;
    }
    
    // Display initial screen
    display_photos_list();
    
    Serial.printf("Photos mode initialized. Found %d photos.\n", photos_count);
}

void handle_photos_mode() {
    // Read button states
    bool up_button_current = button_pressed(BTN_UP);
    bool down_button_current = button_pressed(BTN_DOWN);
    bool select_button_current = button_pressed(BTN_SELECT);
    bool back_button_current = button_pressed(BTN_BACK);
    
    // Handle button presses based on current state
    switch (photos_state) {
        case PHOTOS_LIST:
            // UP button - scroll up in list
            if (up_button_current && !up_button_was_pressed) {
                photos_scroll_up();
            }
            
            // DOWN button - scroll down in list
            if (down_button_current && !down_button_was_pressed) {
                photos_scroll_down();
            }
            
            // SELECT button - view selected photo
            if (select_button_current && !select_button_was_pressed) {
                photos_select_current();
            }
            
            // BACK button - return to menu
            if (back_button_current && !back_button_was_pressed) {
                photos_go_back();
            }
            break;
            
        case PHOTOS_VIEWING:
            // UP button - delete photo
            if (up_button_current && !up_button_was_pressed) {
                photos_state = PHOTOS_CONFIRM_DELETE;
                display_delete_confirmation();
            }
            
            // DOWN button - next photo
            if (down_button_current && !down_button_was_pressed) {
                selected_photo++;
                if (selected_photo >= photos_count) {
                    selected_photo = 0; // Wrap around
                }
                load_and_display_photo(photos_list[selected_photo].filename);
            }
            
            // SELECT button - previous photo  
            if (select_button_current && !select_button_was_pressed) {
                selected_photo--;
                if (selected_photo < 0) {
                    selected_photo = photos_count - 1; // Wrap around
                }
                load_and_display_photo(photos_list[selected_photo].filename);
            }
            
            // BACK button - return to list
            if (back_button_current && !back_button_was_pressed) {
                if (current_photo_buffer) {
                    free(current_photo_buffer);
                    current_photo_buffer = nullptr;
                }
                photos_state = PHOTOS_LIST;
                display_photos_list();
            }
            break;
            
        case PHOTOS_CONFIRM_DELETE:
            // UP button - confirm delete
            if (up_button_current && !up_button_was_pressed) {
                photos_delete_current();
            }
            
            // Any other button - cancel delete
            if (down_button_current && !down_button_was_pressed ||
                select_button_current && !select_button_was_pressed ||
                back_button_current && !back_button_was_pressed) {
                photos_state = PHOTOS_VIEWING;
                load_and_display_photo(photos_list[selected_photo].filename);
            }
            break;
    }
    
    // Update button states
    up_button_was_pressed = up_button_current;
    down_button_was_pressed = down_button_current;
    select_button_was_pressed = select_button_current;
    back_button_was_pressed = back_button_current;
}

void display_photos_list() {
    tft.fillScreen(ST7735_BLACK);
    
    // Title
    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
    tft.setTextSize(1);
    String title = "PHOTOS";
    int title_x = (tft.width() - title.length() * 6) / 2;
    tft.setCursor(title_x, 5);
    tft.print(title);
    
    if (photos_count == 0) {
        show_no_photos_message();
        return;
    }
    
    // Display photo count and page info
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    int current_page = photos_scroll_offset / PHOTOS_PER_PAGE + 1;
    int total_pages = (photos_count + PHOTOS_PER_PAGE - 1) / PHOTOS_PER_PAGE;
    tft.setCursor(5, 17);
    tft.printf("%d photos  Page %d/%d", photos_count, current_page, total_pages);
    
    // Draw photo grid
    draw_photo_grid();
}

void draw_photo_list_item(int index, int y_pos, bool selected) {
    // Clear the line
    tft.fillRect(0, y_pos, tft.width(), 18, ST7735_BLACK);
    
    // Set colors based on selection
    uint16_t bg_color = selected ? ST7735_BLUE : ST7735_BLACK;
    uint16_t text_color = selected ? ST7735_WHITE : ST7735_GREEN;
    
    // Draw selection background
    if (selected) {
        tft.fillRect(2, y_pos, tft.width() - 4, 18, bg_color);
    }
    
    // Draw filename (truncated if too long)
    tft.setTextColor(text_color, bg_color);
    tft.setTextSize(1);
    tft.setCursor(5, y_pos + 2);
    
    String filename = String(photos_list[index].filename);
    if (filename.length() > 18) {
        filename = filename.substring(0, 15) + "...";
    }
    tft.print(filename);
    
    // Draw file size
    char size_str[16];
    format_file_size(photos_list[index].size, size_str, sizeof(size_str));
    tft.setCursor(5, y_pos + 10);
    tft.setTextColor(ST7735_CYAN, bg_color);
    tft.print(size_str);
}

void show_no_photos_message() {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    
    if (!sd_card_is_mounted()) {
        // SD card error message
        String msg1 = "SD Card Error";
        int msg1_x = (tft.width() - msg1.length() * 6) / 2;
        tft.setCursor(msg1_x, 40);
        tft.setTextColor(ST7735_RED, ST7735_BLACK);
        tft.print(msg1);
        
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        String msg2 = "Check connections:";
        int msg2_x = (tft.width() - msg2.length() * 6) / 2;
        tft.setCursor(msg2_x, 60);
        tft.print(msg2);
        
        tft.setCursor(5, 80);
        tft.print("CMD = IO35");
        tft.setCursor(5, 95);
        tft.print("CLK = IO36");
        tft.setCursor(5, 110);
        tft.print("DAT = IO37");
        
    } else {
        // No photos message
        String msg1 = "No photos found";
        int msg1_x = (tft.width() - msg1.length() * 6) / 2;
        tft.setCursor(msg1_x, 60);
        tft.print(msg1);
        
        String msg2 = "Take some photos";
        int msg2_x = (tft.width() - msg2.length() * 6) / 2;
        tft.setCursor(msg2_x, 80);
        tft.print(msg2);
        
        String msg3 = "with camera first!";
        int msg3_x = (tft.width() - msg3.length() * 6) / 2;
        tft.setCursor(msg3_x, 100);
        tft.print(msg3);
    }
}

void photos_scroll_up() {
    if (photos_count == 0) return;
    
    // Move to previous photo (left, then up)
    selected_photo--;
    
    if (selected_photo < 0) {
        // Wrap to last photo
        selected_photo = photos_count - 1;
        photos_scroll_offset = ((photos_count - 1) / PHOTOS_PER_PAGE) * PHOTOS_PER_PAGE;
    } else if (selected_photo < photos_scroll_offset) {
        // Need to scroll up one page
        photos_scroll_offset -= PHOTOS_PER_PAGE;
        if (photos_scroll_offset < 0) photos_scroll_offset = 0;
    }
    
    Serial.printf("UP: selected=%d, offset=%d, total=%d\n", selected_photo, photos_scroll_offset, photos_count);
    display_photos_list();
}

void photos_scroll_down() {
    if (photos_count == 0) return;
    
    // Move to next photo (right, then down)
    selected_photo++;
    
    if (selected_photo >= photos_count) {
        // Wrap to first photo
        selected_photo = 0;
        photos_scroll_offset = 0;
    } else if (selected_photo >= photos_scroll_offset + PHOTOS_PER_PAGE) {
        // Need to scroll down one page
        photos_scroll_offset += PHOTOS_PER_PAGE;
        if (photos_scroll_offset >= photos_count) {
            photos_scroll_offset = ((photos_count - 1) / PHOTOS_PER_PAGE) * PHOTOS_PER_PAGE;
        }
    }
    
    Serial.printf("DOWN: selected=%d, offset=%d, total=%d\n", selected_photo, photos_scroll_offset, photos_count);
    display_photos_list();
}

void photos_select_current() {
    if (photos_count == 0) return;
    
    // Switch to photo viewing mode
    photos_state = PHOTOS_VIEWING;
    load_and_display_photo(photos_list[selected_photo].filename);
}

void photos_go_back() {
    // Clean up and return to menu
    if (current_photo_buffer) {
        free(current_photo_buffer);
        current_photo_buffer = nullptr;
    }
    currentMode = MODE_MENU;
    display_menu();
}

void load_and_display_photo(const char* filename) {
    show_photo_loading();
    
    // Free previous photo buffer
    if (current_photo_buffer) {
        free(current_photo_buffer);
        current_photo_buffer = nullptr;
    }
    
    // Load photo from SD card
    if (load_photo_from_sd(filename, &current_photo_buffer, &current_photo_size)) {
        // Clear screen
        tft.fillScreen(ST7735_BLACK);
        
        // Draw photo title
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(1);
        String title = String(filename);
        if (title.length() > 21) {
            title = title.substring(0, 18) + "...";
        }
        int title_x = (tft.width() - title.length() * 6) / 2;
        tft.setCursor(title_x, 2);
        tft.print(title);
        
        // Decode and display JPEG
        TJpgDec.drawJpg(0, 15, current_photo_buffer, current_photo_size);
        
        // Draw instructions
        tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
        tft.setTextSize(1);
        tft.setCursor(2, 145);
        tft.print("UP:Del DN:Next SEL:Prev");
        tft.setCursor(2, 155);
        tft.print("BACK:List");
        
    } else {
        // Error loading photo
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_RED, ST7735_BLACK);
        tft.setTextSize(1);
        tft.setCursor(10, 60);
        tft.print("Error loading photo");
        tft.setCursor(10, 80);
        tft.print(filename);
        delay(2000);
        
        photos_state = PHOTOS_LIST;
        display_photos_list();
    }
}

void show_photo_loading() {
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    
    String msg = "Loading photo...";
    int msg_x = (tft.width() - msg.length() * 6) / 2;
    tft.setCursor(msg_x, 75);
    tft.print(msg);
}

void display_delete_confirmation() {
    tft.fillScreen(ST7735_BLACK);
    
    // Warning message
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
    tft.setTextSize(1);
    String msg1 = "DELETE PHOTO?";
    int msg1_x = (tft.width() - msg1.length() * 6) / 2;
    tft.setCursor(msg1_x, 50);
    tft.print(msg1);
    
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    String filename = String(photos_list[selected_photo].filename);
    if (filename.length() > 21) {
        filename = filename.substring(0, 18) + "...";
    }
    int filename_x = (tft.width() - filename.length() * 6) / 2;
    tft.setCursor(filename_x, 70);
    tft.print(filename);
    
    tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
    String msg2 = "This cannot be undone!";
    int msg2_x = (tft.width() - msg2.length() * 6) / 2;
    tft.setCursor(msg2_x, 90);
    tft.print(msg2);
    
    // Instructions
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
    tft.setCursor(5, 130);
    tft.print("UP: DELETE");
    
    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
    tft.setCursor(5, 145);
    tft.print("Any other: CANCEL");
}

void photos_delete_current() {
    String filename = String(photos_list[selected_photo].filename);
    
    if (delete_photo_from_sd(filename.c_str())) {
        // Successfully deleted
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
        tft.setTextSize(1);
        String msg = "Photo deleted!";
        int msg_x = (tft.width() - msg.length() * 6) / 2;
        tft.setCursor(msg_x, 75);
        tft.print(msg);
        delay(1000);
        
        // Refresh photos list
        get_photo_list(photos_list, MAX_PHOTOS_LIST, &photos_count);
        
        // Adjust selection if needed
        if (selected_photo >= photos_count && photos_count > 0) {
            selected_photo = photos_count - 1;
        }
        if (photos_scroll_offset > 0 && selected_photo < photos_scroll_offset) {
            photos_scroll_offset = max(0, selected_photo - PHOTOS_PER_PAGE + 1);
        }
        
        photos_state = PHOTOS_LIST;
        display_photos_list();
        
    } else {
        // Error deleting
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_RED, ST7735_BLACK);
        tft.setTextSize(1);
        String msg = "Delete failed!";
        int msg_x = (tft.width() - msg.length() * 6) / 2;
        tft.setCursor(msg_x, 75);
        tft.print(msg);
        delay(2000);
        
        photos_state = PHOTOS_VIEWING;
        load_and_display_photo(photos_list[selected_photo].filename);
    }
}

void draw_photo_grid() {
    int visible_photos = min(PHOTOS_PER_PAGE, photos_count - photos_scroll_offset);
    
    for (int i = 0; i < visible_photos; i++) {
        int photo_index = photos_scroll_offset + i;
        bool is_selected = (photo_index == selected_photo);
        
        draw_thumbnail(i, photo_index, is_selected);
    }
}

void draw_thumbnail(int grid_index, int photo_index, bool selected) {
    // Calculate grid position
    int col = grid_index % GRID_COLS;
    int row = grid_index / GRID_COLS;
    
    int x = 4 + col * (THUMBNAIL_WIDTH + THUMBNAIL_SPACING);
    int y = 30 + row * (THUMBNAIL_HEIGHT + THUMBNAIL_SPACING + 20); // Extra space for filename
    
    // Draw selection border
    uint16_t border_color = selected ? ST7735_YELLOW : ST7735_WHITE;
    uint16_t bg_color = selected ? ST7735_BLUE : ST7735_BLACK;
    
    // Draw border
    tft.drawRect(x - 1, y - 1, THUMBNAIL_WIDTH + 2, THUMBNAIL_HEIGHT + 2, border_color);
    if (selected) {
        tft.drawRect(x - 2, y - 2, THUMBNAIL_WIDTH + 4, THUMBNAIL_HEIGHT + 4, border_color);
    }
    
    // For now, display a colored rectangle based on photo index to show navigation works
    // We'll add real thumbnails once the basic functionality is confirmed
    uint16_t thumb_color;
    switch (photo_index % 6) {
        case 0: thumb_color = ST7735_RED; break;
        case 1: thumb_color = ST7735_GREEN; break;
        case 2: thumb_color = ST7735_BLUE; break;
        case 3: thumb_color = ST7735_YELLOW; break;
        case 4: thumb_color = 0xF81F; break; // Magenta
        default: thumb_color = ST7735_CYAN; break;
    }
    
    // Draw colored thumbnail with photo number
    tft.fillRect(x, y, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, thumb_color);
    
    // Draw photo index number in center
    tft.setTextColor(ST7735_BLACK, thumb_color);
    tft.setTextSize(1);
    String index_str = String(photo_index + 1);
    int index_x = x + (THUMBNAIL_WIDTH - index_str.length() * 6) / 2;
    int index_y = y + THUMBNAIL_HEIGHT / 2 - 4;
    tft.setCursor(index_x, index_y);
    tft.print(index_str);
    
    // Draw filename below thumbnail
    tft.setTextColor(selected ? ST7735_WHITE : ST7735_GREEN, ST7735_BLACK);
    tft.setTextSize(1);
    
    String filename = String(photos_list[photo_index].filename);
    // Truncate filename to fit under thumbnail
    if (filename.length() > 6) {
        filename = filename.substring(0, 5) + "~";
    }
    
    int text_x = x + (THUMBNAIL_WIDTH - filename.length() * 6) / 2;
    int text_y = y + THUMBNAIL_HEIGHT + 3;
    tft.setCursor(text_x, text_y);
    tft.print(filename);
    
    // Draw file size
    char size_str[8];
    format_file_size(photos_list[photo_index].size, size_str, sizeof(size_str));
    String size_string = String(size_str);
    if (size_string.length() > 6) {
        size_string = size_string.substring(0, 5) + "~";
    }
    
    int size_x = x + (THUMBNAIL_WIDTH - size_string.length() * 6) / 2;
    int size_y = y + THUMBNAIL_HEIGHT + 12;
    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);
    tft.setCursor(size_x, size_y);
    tft.print(size_string);
}

void format_file_size(long size, char* buffer, int buffer_size) {
    if (size < 1024) {
        snprintf(buffer, buffer_size, "%ldB", size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%ldK", size / 1024);
    } else {
        snprintf(buffer, buffer_size, "%ldM", size / (1024 * 1024));
    }
}
