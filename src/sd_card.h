#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"

// SD card pin definitions for ESP32-S3
#define SD_MMC_CMD  35   // io35
#define SD_MMC_CLK  36   // io36  
#define SD_MMC_DATA 37   // io37

// Alternative: Try these pins if the above don't work
// #define SD_MMC_CMD  38   
// #define SD_MMC_CLK  39   
// #define SD_MMC_DATA 40   

// Photo storage settings
#define PHOTO_DIR "/photos"
#define MAX_FILENAME_LEN 64
#define MAX_PHOTOS_LIST 50

// Structure to hold photo information
struct PhotoInfo {
    char filename[MAX_FILENAME_LEN];
    size_t size;
    time_t timestamp;
};

// Function declarations
bool sd_card_init();
bool sd_card_mount();
void sd_card_unmount();
bool sd_card_is_mounted();
bool sd_card_test_connection(); // Test function
void sd_card_debug_info(); // Debug information
void sd_card_test_pins(); // Test pin connectivity

// Photo management functions
bool save_photo_to_sd(uint8_t* jpg_buf, size_t jpg_len);
int get_photo_count();
bool get_photo_list(PhotoInfo* photos, int max_count, int* actual_count);
bool load_photo_from_sd(const char* filename, uint8_t** jpg_buf, size_t* jpg_len);
bool delete_photo_from_sd(const char* filename);
String generate_photo_filename();

// SD card status functions
bool check_sd_card_space();
void format_file_size(size_t bytes, char* buffer, size_t buffer_size);

#endif
