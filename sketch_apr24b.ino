#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// WIFI
const char* ssid = "Tuấn P401";
const char* password = "Abzd14578";

WebServer server(80);

// UART ESP -> Arduino
HardwareSerial ArduinoSerial(1);

// ================= CAMERA =================
void startCamera() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;

  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;

  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;

  config.pin_pwdn = 32;
  config.pin_reset = -1;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // 🔥 TỐI ƯU
  config.frame_size = FRAMESIZE_QVGA;   // 320x240
  config.jpeg_quality = 15;             // nhẹ hơn
  config.fb_count = 2;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ CAM FAIL");
    while (true);
  }

  Serial.println("✅ CAM OK");
}

// ================= ROOT =================
void handleRoot() {
  server.send(200, "text/plain", "ESP CAM OK");
}

// ================= STREAM =================
void handleStream() {

  WiFiClient client = server.client();

  String response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";

  server.sendContent(response);

  while (client.connected()) {

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) continue;

    server.sendContent("--frame\r\n");
    server.sendContent("Content-Type: image/jpeg\r\n\r\n");
    server.sendContent((const char*)fb->buf, fb->len);
    server.sendContent("\r\n");

    esp_camera_fb_return(fb);

    // 🔥 QUAN TRỌNG NHẤT
    delay(1);              // nhường CPU
    yield();               // tránh treo
    server.handleClient(); // vẫn nhận /send
  }
}

// ================= SEND =================
void handleSend() {
  if (server.hasArg("label")) {
    String label = server.arg("label");

    Serial.println("Nhan: " + label);

    ArduinoSerial.println(label);

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "No label");
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // UART ESP -> Arduino
  ArduinoSerial.begin(9600, SERIAL_8N1, 15, 14);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nIP:");
  Serial.println(WiFi.localIP());

  startCamera();

  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.on("/send", handleSend);

  server.begin();
}

// ================= LOOP =================
void loop() {
  server.handleClient();
}