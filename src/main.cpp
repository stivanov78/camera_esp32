/*
 * ESP32-CAM Video Streaming Server
 * 
 * This code implements a web server for ESP32-CAM that provides:
 * - Live MJPEG video streaming
 * - Single image capture
 * - Web interface for viewing stream
 * 
 * Hardware: ESP32-CAM (AI-Thinker module)
 * Framework: Arduino
 * 
 * Author: ESP32-CAM Project
 * License: MIT
 */

#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems

// ===========================================
// WiFi Configuration
// ===========================================
// IMPORTANT: Replace with your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===========================================
// Camera Pin Definitions for AI-Thinker ESP32-CAM
// ===========================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===========================================
// Global Variables
// ===========================================
WiFiServer server(80);

// ===========================================
// Function Declarations
// ===========================================
void startCameraServer();
void handleRoot(WiFiClient &client);
void handleStream(WiFiClient &client);
void handleCapture(WiFiClient &client);

// ===========================================
// Setup Function
// ===========================================
void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("ESP32-CAM Video Streaming Server");
  Serial.println("================================");

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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Check if PSRAM is available
  if(psramFound()){
    Serial.println("PSRAM found! Using UXGA resolution");
    config.frame_size = FRAMESIZE_UXGA;   // 1600x1200
    config.jpeg_quality = 10;              // 0-63, lower means higher quality
    config.fb_count = 2;                   // Number of frame buffers
  } else {
    Serial.println("PSRAM not found! Using SVGA resolution");
    config.frame_size = FRAMESIZE_SVGA;    // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    Serial.println("Please check:");
    Serial.println("1. Camera connections");
    Serial.println("2. Power supply (needs stable 5V)");
    Serial.println("3. GPIO pins configuration");
    return;
  }

  Serial.println("Camera initialized successfully!");

  // Get camera sensor and apply settings
  sensor_t * s = esp_camera_sensor_get();
  if (s == NULL) {
    Serial.println("Failed to get camera sensor!");
    return;
  }

  // Configure camera settings for optimal streaming
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

  Serial.println("Camera settings applied");

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 40) {
      Serial.println();
      Serial.println("Failed to connect to WiFi!");
      Serial.println("Please check:");
      Serial.println("1. SSID and password are correct");
      Serial.println("2. WiFi network is available");
      Serial.println("3. ESP32 is in range");
      return;
    }
  }

  Serial.println();
  Serial.println("WiFi connected successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("Available endpoints:");
  Serial.print("- Web Interface:  http://");
  Serial.println(WiFi.localIP());
  Serial.print("- Video Stream:   http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");
  Serial.print("- Capture Image:  http://");
  Serial.print(WiFi.localIP());
  Serial.println("/capture");
  Serial.println();

  // Start web server
  server.begin();
  Serial.println("Web server started!");
}

// ===========================================
// Loop Function
// ===========================================
void loop() {
  // Check for client connections
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    String request = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        
        if (c == '\n') {
          // If the current line is blank, you got two newline characters in a row
          // That's the end of the HTTP request, so send a response
          if (currentLine.length() == 0) {
            // Parse the request
            if (request.indexOf("GET / ") >= 0) {
              handleRoot(client);
            } 
            else if (request.indexOf("GET /stream") >= 0) {
              handleStream(client);
            }
            else if (request.indexOf("GET /capture") >= 0) {
              handleCapture(client);
            }
            else {
              // Send 404 Not Found
              client.println("HTTP/1.1 404 Not Found");
              client.println("Content-type: text/html");
              client.println();
              client.println("<!DOCTYPE HTML><html><body><h1>404 Not Found</h1></body></html>");
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}

// ===========================================
// Handle Root Page
// ===========================================
void handleRoot(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: text/html");
  client.println();
  
  // HTML page
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>ESP32-CAM Video Stream</title>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background: #f0f0f0; }");
  client.println(".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }");
  client.println("h1 { color: #333; margin-bottom: 10px; }");
  client.println("img { max-width: 100%; height: auto; border: 2px solid #ddd; border-radius: 5px; margin: 20px 0; }");
  client.println("button { background: #007bff; color: white; border: none; padding: 12px 30px; font-size: 16px; border-radius: 5px; cursor: pointer; margin: 5px; }");
  client.println("button:hover { background: #0056b3; }");
  client.println(".info { color: #666; font-size: 14px; margin: 10px 0; }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class='container'>");
  client.println("<h1>ESP32-CAM Video Streaming</h1>");
  client.println("<p class='info'>Live video stream from ESP32-CAM module</p>");
  client.println("<img id='stream' src='/stream' />");
  client.println("<div>");
  client.println("<button onclick='location.reload()'>Refresh</button>");
  client.println("<button onclick='captureImage()'>Capture Image</button>");
  client.println("</div>");
  client.println("<p class='info'>Stream endpoint: /stream</p>");
  client.println("<p class='info'>Capture endpoint: /capture</p>");
  client.println("</div>");
  client.println("<script>");
  client.println("function captureImage() {");
  client.println("  window.open('/capture', '_blank');");
  client.println("}");
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");
}

// ===========================================
// Handle Video Stream
// ===========================================
void handleStream(WiFiClient &client) {
  Serial.println("Starting video stream...");
  
  // Send HTTP headers for MJPEG stream
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();
  
  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }
    
    // Send MJPEG frame
    client.println("--frame");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();
    client.write(fb->buf, fb->len);
    client.println();
    
    esp_camera_fb_return(fb);
    
    // Frame rate control (~33 FPS with 30ms delay)
    delay(30);
    
    // Check if client is still connected
    if (!client.connected()) {
      break;
    }
  }
  
  Serial.println("Video stream ended");
}

// ===========================================
// Handle Single Image Capture
// ===========================================
void handleCapture(WiFiClient &client) {
  Serial.println("Capturing single image...");
  
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Content-type: text/html");
    client.println();
    client.println("<!DOCTYPE HTML><html><body><h1>Camera Capture Failed</h1></body></html>");
    return;
  }
  
  // Send HTTP headers for JPEG image
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.print("Content-Length: ");
  client.println(fb->len);
  client.println("Content-Disposition: inline; filename=capture.jpg");
  client.println();
  
  // Send image data
  client.write(fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
  Serial.println("Image captured and sent successfully");
}
