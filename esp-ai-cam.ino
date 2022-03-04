#include "who_camera.h"
#include "camera_server.hpp"
#include "who_human_face_detection.hpp"
#include "pins.h"
#include "ws_server.h"
#include "wifi_helper.h"

// for production, uncomment this line
#define DEBUG

#define CAMERA_VERTICAL_FLIP 1

WS_Server ws_server = WS_Server();
WiFiHelper wifi = WiFiHelper();

String ssid, password;
int mode;
String rxBuf = "";
bool is_stream_on = false;

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueHttpFrame = NULL;
static QueueHandle_t xQueueAIData = NULL;
bool is_camera_started = false;

// Functions
void camera_init()
void ai_data_handler()
String serialRead();
void handleSet(String cmd);

void setup() {
  Serial.begin(115200);
  // Serial.setTimeout(SERIAL_TIMEOUT);

  #ifdef DEBUG
  Serial.println("[DEBUG] Start!");
  #endif
  Serial.println("\r\n[OK]");
}

void loop() {
  rxBuf = serialRead();
  if (rxBuf.length() > 0) {
    #ifdef DEBUG
    Serial.print("[DEBUG] RX Receive: ");Serial.println(rxBuf);
    #endif
    delay(10);
    if (rxBuf.substring(0, 4) == "SET+"){
      handleSet(rxBuf.substring(4));
    } else if (rxBuf.substring(0, 3) == "WS+"){
      String out = rxBuf.substring(3);
      #ifdef DEBUG
      Serial.print("[DEBUG] Read from Serial: ");Serial.println(out);
      #endif
      ws_server.send(out);
    }
  }
  ws_server.loop();
  if (ws_server.is_connected()){
    camera_init();
  }
  if (is_camera_started) {
    ai_data_handler();
  }
}

void camera_init(){
  if (is_camera_started) {
    return;
  }
  // xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  // xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  // xQueueAIData = xQueueCreate(2, sizeof(int) * 4);
  xQueueAIFrame = xQueueCreate(1, sizeof(camera_fb_t *));
  xQueueHttpFrame = xQueueCreate(1, sizeof(camera_fb_t *));
  xQueueAIData = xQueueCreate(1, sizeof(int) * 4);
    
  register_camera(
      PIXFORMAT_RGB565, FRAMESIZE_QVGA, 1, xQueueAIFrame, CAMERA_VERTICAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  if (is_stream_on){
    register_human_face_detection(xQueueAIFrame, NULL, xQueueAIData, xQueueHttpFrame, true);
    register_httpd(xQueueHttpFrame, NULL, true);
  } else {
    register_human_face_detection(xQueueAIFrame, NULL, xQueueAIData, NULL, true);
  }
  is_camera_started = true;
}

void ai_data_handler() {
  uint8_t data[4];
  if (xQueueReceive(xQueueAIData, &data, portMAX_DELAY)) {
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
  while (Serial.available() && millis() - timeoutStart < SERIAL_TIMEOUT) {
    temp = Serial.read();
    inChar = (char)temp;
    if (inChar == '\n') {
      break;
    } else if((int)inChar != 255){
      buf += inChar;
    }
  }
  return buf;
}

void handleSet(String cmd){
  if (cmd.substring(0, 4) == "SSID"){
    ssid = cmd.substring(4);
    #ifdef DEBUG
    Serial.print("[DEBUG] Set SSID: ");Serial.println(ssid);
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 3) == "PSK"){
    password = cmd.substring(3);
    #ifdef DEBUG
    Serial.print("[DEBUG] Set password: ");Serial.println(password);
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "PORT"){
    port = cmd.substring(4).toInt();
    #ifdef DEBUG
    Serial.print("[DEBUG] Set port: ");Serial.println(port);
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "MODE"){
    mode = cmd.substring(4).toInt();
    #ifdef DEBUG
    Serial.print("[DEBUG] Set mode: ");
    if (mode == AP){
      Serial.println("AP");
    } else if (mode == STA) {
      Serial.println("STA");
    }
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 5) == "RESET"){
    #ifdef DEBUG
    Serial.println("[DEBUG] Reset");
    #endif
    delay(10);
    ESP.restart();
  } else if (cmd.substring(0, 5) == "START"){
    if (ssid.length() == 0) {
      Serial.println("[ERROR] Please set ssid");
    } else if (password.length() == 0) {
      Serial.println("[ERROR] Please set password");
    } else if (mode == NONE) {
      Serial.println("[ERROR] Please set mode");
    } else if (port == 0) {
      Serial.println("[ERROR] Please set port");
    } else{
      bool result = wifi.connect(mode, ssid, password);
      if (!result) {
        Serial.println("[ERROR] TIMEOUT");
      } else {
        ws_server.begin(port);
        #ifdef DEBUG
        Serial.println("[DEBUG] Websocket on!");
        #endif
        Serial.print("[OK] ");Serial.println(wifi.ip);
      }
    }
  } else if (cmd.substring(0, 6) == "STREAM"){
    if (cmd.substring(6, 7) == "1"){
      is_stream_on = true;
      #ifdef DEBUG
      Serial.println("[DEBUG] Start stream");
      #endif
      Serial.println("[OK]");
    } else if (cmd.substring(6, 7) == "0"){
      is_stream_on = false;
      #ifdef DEBUG
      Serial.println("[DEBUG] Stop stream");
      #endif
      Serial.println("[OK]");
    }
  } else {
    Serial.println("[ERROR] Unknown command");
  }
}
