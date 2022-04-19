#include "who_camera.h"
#include "camera_server.hpp"
#include "who_human_face_detection.hpp"
#include "who_color_detection.hpp"
#include "ws_server.h"
#include "wifi_helper.h"

#define ESP32_CAM
// #define TTGO_CAMERA
#include "pins.h"

// for production, uncomment this line
// #define DEBUG

#define SERIAL_TIMEOUT 100
#define CAMERA_VERTICAL_FLIP 1
#define CAMERA_MODE_AI 0
#define CAMERA_MODE_STREAM 1
#define CAMERA_MODE_BOTH 2
#define STATUS_LED 2

String CAMERA_MODES[3] = {"AI", "Stream", "AI&Stream"};
String WIFI_MODES[3] = {"None", "STA", "AP"};

WS_Server ws_server = WS_Server();
WiFiHelper wifi = WiFiHelper();

String ssid = "AI_Camera";
String password = "12345678";
int port = 8765;
int mode = AP;
String rxBuf = "";
double startMillis = 0;

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueHttpFrame = NULL;
static QueueHandle_t xQueueAIData = NULL;
bool is_camera_started = false;
int camera_mode = CAMERA_MODE_STREAM;

// Functions
void camera_init();
void ai_data_handler();
String serialRead();
void handleSet(String cmd);
void debug(String msg);
void debug(String msg, int data);
void debug(String msg, String data);
void error(String msg);
void error(String msg, int data);
void error(String msg, String data);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(SERIAL_TIMEOUT);

  pinMode(STATUS_LED, OUTPUT);
  debug("Start!");
  Serial.println("\r\n[OK]");
}

void loop() {
  rxBuf = serialRead();
  if (rxBuf.length() > 0) {
    debug("RX Receive: ", rxBuf);
    // delay(10);
    if (rxBuf.substring(0, 4) == "SET+") {
      handleSet(rxBuf.substring(4));
    } else if (rxBuf.substring(0, 3) == "WS+") {
      String out = rxBuf.substring(3);
      debug("Read from Serial: ", out);
      ws_server.send(out);
    }
  }
  if (millis() - startMillis > 1000) {
    startMillis = millis();
    digitalWrite(STATUS_LED, 1);
  }
  if (wifi.is_connected) {
    ws_server.loop();
    if (!is_camera_started){
      camera_init();
    }
  } else {
    if (millis() - startMillis > 2000) {
      startMillis = millis();
      digitalWrite(STATUS_LED, 0);
    }
  }
  if (is_camera_started && (camera_mode == CAMERA_MODE_AI || camera_mode == CAMERA_MODE_BOTH)) {
    ai_data_handler();
  }
}

void camera_init(){
  xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueAIData = xQueueCreate(2, sizeof(int) * 4);

  pixformat_t pixel_format = PIXFORMAT_JPEG;
  if (camera_mode == CAMERA_MODE_AI || camera_mode == CAMERA_MODE_BOTH) {
    pixel_format = PIXFORMAT_RGB565;
  } else {
    pixel_format = PIXFORMAT_JPEG;
  }
  register_camera(
      pixel_format, FRAMESIZE_QVGA, 1, xQueueAIFrame, CAMERA_VERTICAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  switch (camera_mode) {
    case CAMERA_MODE_AI:
      // register_color_detection(xQueueAIFrame, NULL, NULL, NULL, true);
      register_human_face_detection(xQueueAIFrame, NULL, xQueueAIData, NULL, true);
      break;
    case CAMERA_MODE_STREAM:
      register_httpd(xQueueAIFrame, NULL, true);
      break;
    case CAMERA_MODE_BOTH:
      // register_color_detection(xQueueAIFrame, NULL, NULL, xQueueHttpFrame, true);
      register_human_face_detection(xQueueAIFrame, NULL, xQueueAIData, xQueueHttpFrame, true);
      register_httpd(xQueueHttpFrame, NULL, true);
      break;
    default:
      error("Unknown camera mode: " + String(camera_mode));
      while (1);
      break;
  }
  is_camera_started = true;
}

void ai_data_handler() {
  uint8_t data[4];
  if (xQueueReceive(xQueueAIData, &data, 0)) {
    Serial.print("AI+[");
    for (int i = 0; i < 4; i++) {
      Serial.print(data[i]);
      if (i != 3) {
        Serial.print(", ");
      }
    }
    Serial.println("]");
  }
}

String serialRead() {
  String buf = "";
  char inChar;
  int temp;
  unsigned long timeoutStart = millis();
  while (Serial.available() || millis() - timeoutStart < SERIAL_TIMEOUT) {
    temp = Serial.read();
    inChar = (char)temp;
    if (inChar == '\n') {
      break;
    } else if (inChar == '\r') {
      continue;
    } else if ((int)inChar != 255) {
      buf += inChar;
    }
  }
  return buf;
}

void handleSet(String cmd){
  if (cmd.substring(0, 4) == "SSID"){
    ssid = cmd.substring(4);
    debug("Set SSID: ", ssid);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 3) == "PSK"){
    password = cmd.substring(3);
    debug("Set password: ", password);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "PORT"){
    port = cmd.substring(4).toInt();
    debug("Set port: ", port);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "MODE"){
    mode = cmd.substring(4).toInt();
    debug("Set mode: ", WIFI_MODES[mode]);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 5) == "RESET"){
    debug("Reset");
    delay(10);
    ESP.restart();
  } else if (cmd.substring(0, 5) == "START"){
    start();
  } else if (cmd.substring(0, 11) == "CAMERA_MODE") {
    if (is_camera_started) {
      error("Camera is already started");
    } else {
      camera_mode = cmd.substring(11).toInt();
      debug("Set camera mode: ", CAMERA_MODES[camera_mode]);
      Serial.println("[OK]");
    }
  } else {
    error("Unknown command");
  }
}

void debug(String msg){
  #ifdef DEBUG
  msg = "[DEBUG] " + msg;
  Serial.println(msg);
  #endif
}

void debug(String msg, int data){
  msg = msg + String(data);
  debug(msg);
}

void debug(String msg, String data){
  msg = msg + String(data);
  debug(msg);
}

void error(String msg){
  msg = "[ERROR] " + msg;
  Serial.println(msg);
}

void error(String msg, int data){
  msg = msg + String(data);
  error(msg);
}

void error(String msg, String data){
  msg = msg + String(data);
  error(msg);
}

void start() {
  if (ssid.length() == 0) {
    error("Please set ssid");
  } else if (password.length() == 0) {
    error("Please set password");
  } else if (mode == NONE) {
    error("Please set mode");
  } else if (port == 0) {
    error("Please set port");
  } else{
    bool result = wifi.connect(mode, ssid, password);
    if (!result) {
      error("TIMEOUT");
    } else {
      ws_server.begin(port);
      debug("Websocket on!");
      Serial.print("[OK] ");Serial.println(wifi.ip);
    }
  }
}