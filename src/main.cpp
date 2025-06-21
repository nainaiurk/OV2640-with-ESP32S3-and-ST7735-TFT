#include <Arduino.h>
#include <esp_camera.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <TJpg_Decoder.h>
#include <Wire.h>

// TFT pin definitions
#define TFT_CS   10  // Chip Select
#define TFT_RST  9   // Reset
#define TFT_DC   3   // Data/Command
#define TFT_MOSI 11  // DIN (Data In)
#define TFT_SCLK 12  // CLK (Clock)

// Camera pin configuration
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   20
#define XCLK_GPIO_NUM    42
#define SIOD_GPIO_NUM    6
#define SIOC_GPIO_NUM    7
#define Y9_GPIO_NUM      45
#define Y8_GPIO_NUM      38
#define Y7_GPIO_NUM      48
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      47
#define Y4_GPIO_NUM      40
#define Y3_GPIO_NUM      21
#define Y2_GPIO_NUM      41
#define VSYNC_GPIO_NUM   46
#define HREF_GPIO_NUM    44
#define PCLK_GPIO_NUM    15

// Initialize TFT
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// JPEG decoder callback function
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data) {
  for (uint16_t j = 0; j < h && y + j < 160; j++) {
    for (uint16_t i = 0; i < w && x + i < 128; i++) {
      tft.drawPixel(x + i, y + j, data[j * w + i]);
    }
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Starting setup...");

  // Test TFT
  tft.initR(INITR_REDTAB); // Try INITR_GREENTAB or INITR_BLACKTAB if colors off
  tft.setRotation(2); // Adjust rotation (0-3)
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.println("TFT Initialized");
  tft.println("Testing display...");
  delay(1000);

  // Draw test pattern
  tft.fillScreen(ST7735_BLACK);
  tft.drawRect(0, 0, 128, 160, ST7735_RED);
  tft.drawLine(0, 0, 127, 159, ST7735_GREEN);
  tft.drawLine(0, 159, 127, 0, ST7735_BLUE);
  tft.setCursor(10, 70);
  tft.println("TFT Test OK");
  delay(2000);
  tft.fillScreen(ST7735_BLACK);

  // Check I2C with retries
  // if (!checkI2C()) {
  //   Serial.println("OV2640 NOT detected on I2C");
  //   tft.fillScreen(ST7735_RED);
  //   tft.setCursor(0, 0);
  //   tft.println("OV2640 I2C fail");
  //   delay(5000);
  //   return;
  // }
  // Serial.println("OV2640 detected on I2C");
  // tft.println("OV2640 I2C OK");
  // delay(1000);
  // tft.fillScreen(ST7735_BLACK);
  // tft.setCursor(0, 0);
  // tft.println("Initializing camera...");

  // Camera configuration (from working Wi-Fi code)
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
  config.xclk_freq_hz = 20000000; // 10 MHz
  config.frame_size = FRAMESIZE_QVGA; // 320x240
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  // config.fb_location = CAMERA_FB_IN_DRAM; // Use DRAM
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 30; // Lower quality
  config.fb_count = 1;

  // Initialize camera
  Serial.println("Initializing camera...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    tft.fillScreen(ST7735_RED);
    tft.setCursor(0, 0);
    tft.printf("Camera init failed: 0x%x", err);
    return;
  }
  Serial.println("Camera initialized successfully!");

  sensor_t *s = esp_camera_sensor_get();
  s->set_hmirror(s, 1); // No horizontal flip (0=disable, 1=enable)
  s->set_vflip(s, 0); // No vertical flip (0=disable, 1=enable)

  tft.fillScreen(ST7735_GREEN);
  tft.setCursor(0, 0);
  tft.println("Camera initialized!");
  delay(2000);
  tft.fillScreen(ST7735_BLACK);

  // Initialize JPEG decoder
  TJpgDec.setJpgScale(2); // Scale 320x240 to ~160x120
  TJpgDec.setCallback(tft_output);
  Serial.println("JPEG decoder initialized");
}

void loop() {
  static uint32_t frame_count = 0;
  static uint32_t last_time = 0;

  // Capture frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    tft.fillScreen(ST7735_RED);
    tft.setCursor(0, 0);
    tft.println("Capture failed");
    delay(1000);
    tft.fillScreen(ST7735_BLACK);
    return;
  }

  // Decode and display JPEG
  if (fb->len > 0) {
    TJpgDec.drawJpg(0, 0, fb->buf, fb->len); // Draw at (0,0)
  }

  // Return frame buffer
  esp_camera_fb_return(fb);

  // Calculate FPS
  frame_count++;
  if (millis() - last_time >= 1000) {
    Serial.printf("FPS: %u\n", frame_count);
    frame_count = 0;
    last_time = millis();
  }
}