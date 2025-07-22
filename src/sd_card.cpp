#include "sd_card.h"
#include "FS.h"
#include "SD_MMC.h"
#include <time.h>

static bool sd_mounted = false;

bool sd_card_init() {
    Serial.println("Initializing SD card...");
    
    // Configure SD_MMC pins
    Serial.printf("Setting SD_MMC pins: CLK=%d, CMD=%d, DATA=%d\n", SD_MMC_CLK, SD_MMC_CMD, SD_MMC_DATA);
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_DATA);
    
    // Try to mount SD card with more detailed error reporting
    Serial.println("Attempting to mount SD card in 1-bit mode...");
    if (!SD_MMC.begin("/sdcard", true)) { // true = 1-bit mode for single data line
        Serial.println("SD Card Mount Failed - trying without mount point...");
        
        // Try alternative mounting
        if (!SD_MMC.begin()) {
            Serial.println("SD Card Mount Failed completely");
            return false;
        }
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    
    // Create photos directory if it doesn't exist
    if (!SD_MMC.exists(PHOTO_DIR)) {
        if (SD_MMC.mkdir(PHOTO_DIR)) {
            Serial.println("Created /photos directory");
        } else {
            Serial.println("Failed to create /photos directory");
            return false;
        }
    }
    
    sd_mounted = true;
    Serial.println("SD card initialized successfully");
    return true;
}

bool sd_card_mount() {
    if (sd_mounted) return true;
    return sd_card_init();
}

void sd_card_unmount() {
    if (sd_mounted) {
        SD_MMC.end();
        sd_mounted = false;
        Serial.println("SD card unmounted");
    }
}

bool sd_card_is_mounted() {
    return sd_mounted;
}

bool sd_card_test_connection() {
    Serial.println("=== SD Card Connection Test ===");
    Serial.printf("Pins: CMD=%d, CLK=%d, DATA=%d\n", SD_MMC_CMD, SD_MMC_CLK, SD_MMC_DATA);
    
    // Test 1: Try basic initialization
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_DATA);
    
    Serial.println("Test 1: Basic SD_MMC.begin()");
    if (SD_MMC.begin()) {
        Serial.println("✓ Basic init successful");
        uint8_t cardType = SD_MMC.cardType();
        Serial.printf("Card type: %d\n", cardType);
        SD_MMC.end();
        return true;
    } else {
        Serial.println("✗ Basic init failed");
    }
    
    Serial.println("Test 2: SD_MMC.begin with 1-bit mode");
    if (SD_MMC.begin("/sdcard", true)) {
        Serial.println("✓ 1-bit mode successful");
        SD_MMC.end();
        return true;
    } else {
        Serial.println("✗ 1-bit mode failed");
    }
    
    Serial.println("Test 3: SD_MMC.begin with 4-bit mode");
    if (SD_MMC.begin("/sdcard", false)) {
        Serial.println("✓ 4-bit mode successful");
        SD_MMC.end();
        return true;
    } else {
        Serial.println("✗ 4-bit mode failed");
    }
    
    Serial.println("=== All SD card tests failed ===");
    return false;
}

void sd_card_debug_info() {
    Serial.println("=== SD CARD DEBUG INFO ===");
    Serial.printf("SD card mounted: %s\n", sd_mounted ? "YES" : "NO");
    Serial.printf("Pin configuration:\n");
    Serial.printf("  CMD = IO%d\n", SD_MMC_CMD);
    Serial.printf("  CLK = IO%d\n", SD_MMC_CLK);
    Serial.printf("  DATA = IO%d\n", SD_MMC_DATA);
    
    if (sd_mounted) {
        uint8_t cardType = SD_MMC.cardType();
        Serial.printf("Card Type: ");
        switch(cardType) {
            case CARD_NONE: Serial.println("NONE"); break;
            case CARD_MMC: Serial.println("MMC"); break;
            case CARD_SD: Serial.println("SD"); break;
            case CARD_SDHC: Serial.println("SDHC"); break;
            default: Serial.printf("UNKNOWN (%d)\n", cardType); break;
        }
        
        uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
        Serial.printf("Card Size: %lluMB\n", cardSize);
        
        uint64_t totalBytes = SD_MMC.totalBytes();
        uint64_t usedBytes = SD_MMC.usedBytes();
        Serial.printf("Total: %lluMB, Used: %lluMB, Free: %lluMB\n", 
                      totalBytes / (1024 * 1024), 
                      usedBytes / (1024 * 1024), 
                      (totalBytes - usedBytes) / (1024 * 1024));
        
        // Check if photos directory exists
        if (SD_MMC.exists(PHOTO_DIR)) {
            Serial.printf("Photos directory: EXISTS\n");
            int photo_count = get_photo_count();
            Serial.printf("Photos found: %d\n", photo_count);
        } else {
            Serial.printf("Photos directory: MISSING\n");
        }
    } else {
        Serial.println("Cannot get card info - not mounted");
    }
    Serial.println("=========================");
}

void sd_card_test_pins() {
    Serial.println("=== SD CARD PIN TEST ===");
    Serial.printf("Testing pins: CMD=%d, CLK=%d, DATA=%d\n", SD_MMC_CMD, SD_MMC_CLK, SD_MMC_DATA);
    
    // Set pins as input with pullup and read them
    pinMode(SD_MMC_CMD, INPUT_PULLUP);
    pinMode(SD_MMC_CLK, INPUT_PULLUP);
    pinMode(SD_MMC_DATA, INPUT_PULLUP);
    
    delay(100); // Let pins settle
    
    bool cmd_state = digitalRead(SD_MMC_CMD);
    bool clk_state = digitalRead(SD_MMC_CLK);
    bool data_state = digitalRead(SD_MMC_DATA);
    
    Serial.printf("Pin states (HIGH=1, LOW=0):\n");
    Serial.printf("  CMD (IO%d): %s\n", SD_MMC_CMD, cmd_state ? "HIGH" : "LOW");
    Serial.printf("  CLK (IO%d): %s\n", SD_MMC_CLK, clk_state ? "HIGH" : "LOW");
    Serial.printf("  DATA (IO%d): %s\n", SD_MMC_DATA, data_state ? "HIGH" : "LOW");
    
    // Check for expected states (should be HIGH with pullup if not connected to anything)
    if (cmd_state && clk_state && data_state) {
        Serial.println("Status: Pins are floating HIGH (no SD card connected or bad connection)");
    } else {
        Serial.println("Status: Some pins are LOW (possible SD card connection detected)");
    }
    
    Serial.println("========================");
}

String generate_photo_filename() {
    // Generate filename with timestamp
    time_t now = time(nullptr);
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    
    char filename[MAX_FILENAME_LEN];
    snprintf(filename, sizeof(filename), "/photos/IMG_%04d%02d%02d_%02d%02d%02d.jpg",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    return String(filename);
}

bool save_photo_to_sd(uint8_t* jpg_buf, size_t jpg_len) {
    if (!sd_mounted) {
        Serial.println("SD card not mounted");
        return false;
    }
    
    if (!jpg_buf || jpg_len == 0) {
        Serial.println("Invalid photo data");
        return false;
    }
    
    String filename = generate_photo_filename();
    
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    
    size_t bytes_written = file.write(jpg_buf, jpg_len);
    file.close();
    
    if (bytes_written != jpg_len) {
        Serial.printf("Write error: wrote %d bytes, expected %d\n", bytes_written, jpg_len);
        return false;
    }
    
    Serial.printf("Photo saved: %s (%d bytes)\n", filename.c_str(), jpg_len);
    return true;
}

int get_photo_count() {
    if (!sd_mounted) return 0;
    
    File dir = SD_MMC.open(PHOTO_DIR);
    if (!dir) return 0;
    
    int count = 0;
    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory() && String(file.name()).endsWith(".jpg")) {
            count++;
        }
        file.close();
        file = dir.openNextFile();
    }
    dir.close();
    
    return count;
}

bool get_photo_list(PhotoInfo* photos, int max_count, int* actual_count) {
    if (!sd_mounted || !photos || !actual_count) return false;
    
    *actual_count = 0;
    File dir = SD_MMC.open(PHOTO_DIR);
    if (!dir) return false;
    
    File file = dir.openNextFile();
    while (file && *actual_count < max_count) {
        if (!file.isDirectory() && String(file.name()).endsWith(".jpg")) {
            // Copy filename (remove directory path)
            String fullPath = String(file.name());
            String justFilename = fullPath.substring(fullPath.lastIndexOf('/') + 1);
            strncpy(photos[*actual_count].filename, justFilename.c_str(), MAX_FILENAME_LEN - 1);
            photos[*actual_count].filename[MAX_FILENAME_LEN - 1] = '\0';
            
            photos[*actual_count].size = file.size();
            photos[*actual_count].timestamp = file.getLastWrite();
            
            (*actual_count)++;
        }
        file.close();
        file = dir.openNextFile();
    }
    dir.close();
    
    return true;
}

bool load_photo_from_sd(const char* filename, uint8_t** jpg_buf, size_t* jpg_len) {
    if (!sd_mounted || !filename || !jpg_buf || !jpg_len) return false;
    
    String fullPath = String(PHOTO_DIR) + "/" + String(filename);
    File file = SD_MMC.open(fullPath.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("Failed to open photo: %s\n", fullPath.c_str());
        return false;
    }
    
    *jpg_len = file.size();
    *jpg_buf = (uint8_t*)malloc(*jpg_len);
    if (!*jpg_buf) {
        Serial.println("Failed to allocate memory for photo");
        file.close();
        return false;
    }
    
    size_t bytes_read = file.read(*jpg_buf, *jpg_len);
    file.close();
    
    if (bytes_read != *jpg_len) {
        Serial.printf("Read error: read %d bytes, expected %d\n", bytes_read, *jpg_len);
        free(*jpg_buf);
        *jpg_buf = nullptr;
        return false;
    }
    
    Serial.printf("Photo loaded: %s (%d bytes)\n", filename, *jpg_len);
    return true;
}

bool delete_photo_from_sd(const char* filename) {
    if (!sd_mounted || !filename) return false;
    
    String fullPath = String(PHOTO_DIR) + "/" + String(filename);
    if (SD_MMC.remove(fullPath.c_str())) {
        Serial.printf("Photo deleted: %s\n", filename);
        return true;
    } else {
        Serial.printf("Failed to delete photo: %s\n", filename);
        return false;
    }
}

bool check_sd_card_space() {
    if (!sd_mounted) return false;
    
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    uint64_t freeBytes = totalBytes - usedBytes;
    
    Serial.printf("SD Card - Total: %lluMB, Used: %lluMB, Free: %lluMB\n", 
                  totalBytes / (1024 * 1024), 
                  usedBytes / (1024 * 1024), 
                  freeBytes / (1024 * 1024));
    
    // Return true if at least 10MB free
    return (freeBytes > (10 * 1024 * 1024));
}

void format_file_size(size_t bytes, char* buffer, size_t buffer_size) {
    if (bytes < 1024) {
        snprintf(buffer, buffer_size, "%d B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f KB", bytes / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%.1f MB", bytes / (1024.0 * 1024.0));
    }
}
