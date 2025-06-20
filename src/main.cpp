#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// Access Point credentials
const char* ap_ssid = "ESP32-CAM-AP";
const char* ap_password = "12345678"; // Minimum 8 characters

// Web server on port 80
WebServer server(80);

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

// Handler for MJPEG streaming
void handle_stream() {
  WiFiClient client = server.client();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }
    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n\r\n");
    client.write((char *)fb->buf, fb->len);
    client.print("\r\n");
    esp_camera_fb_return(fb);
    delay(50); // Adjust for frame rate
  }
}

// Handler for root URL (/)
void handle_root() {
  String html = "<!DOCTYPE html><html><head><title>ESP32-S3 Camera</title></head>";
  html += "<body><h1>ESP32-S3 Camera Stream</h1>";
  html += "<p><a href=\"/stream\">View Camera Stream</a></p>";
  html += "<p>Connect to Wi-Fi: " + String(ap_ssid) + "</p>";
  html += "<p>Access via: <a href=\"http://esp32cam.local/stream\">http://esp32cam.local/stream</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Camera configuration
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
  config.xclk_freq_hz = 10000000; // 10 MHz
  config.frame_size = FRAMESIZE_QVGA; // 320x240
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_DRAM; // Use DRAM
  config.jpeg_quality = 20; // Lower quality
  config.fb_count = 1;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
  Serial.println("Camera initialized successfully!");

  // Set up Access Point
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Set up mDNS
  if (!MDNS.begin("esp32cam")) {
    Serial.println("Error starting mDNS");
    return;
  }
  Serial.println("mDNS started: http://esp32cam.local");

  // Set up web server routes
  server.on("/", handle_root); // Handle root URL
  server.on("/stream", handle_stream); // Handle stream URL
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}