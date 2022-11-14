/*******************************************************************
  Camera firmware for serial communication with the lower computer，
  and wenbsocket communication with the Sunfounder Controller APP,
  targeted at ESP32-CAM and TTGO_CAMERA.

  Development test environment:
    - Arduino IDE 1.8.19
  Board tools:
    - Arduino AVR Boards 1.8.3
    - esp32 2.0.3
  Libraries:
    - ArduinoJson
    - WebSockets

  Version: 1.0.0
    -- https://github.com/sunfounder/ai-camera-firmware
  
  Author: Sunfounder
  Website: http://www.sunfounder.com
           https://docs.sunfounder.com
 *******************************************************************/

#include "led_status.hpp"
#include "who_camera.h"
#include "camera_server.hpp"
#include "ws_server.h"
#include "wifi_helper.h"
#include "ArduinoJson.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

/*
  Select development board
*/
#define ESP32_CAM
// #define TTGO_CAMERA
#include "pins.h"

/*
  Set Wifi mode, SSID and password
*/
int mode = AP;  // STA or AP
String ssid = "aiCAM";
String password = "12345678";

// int mode = STA;  // STA or AP
// String ssid = "xxxxxx";
// String password = "xxxxxxxx";

/*
  Set websockets port
*/
int port = 8765;  // Sunfounder Controller APP fixed using port 8765

/*
  Set check info for Sunfounder Controller APP
*/
String name = "AI Camera";
String type = "AI_Camera";
String videoUrl = "";
#define CHECK_TEXT "SC"

/*
  Set the Debug Level
*/
#define DEBUG_LEVEL CAM_DEBUG_LEVEL_INFO
#define CAM_DEBUG_LEVEL_OFF 0
#define CAM_DEBUG_LEVEL_ERROR 1
#define CAM_DEBUG_LEVEL_INFO 2
#define CAM_DEBUG_LEVEL_DEBUG 3
#define CAM_DEBUG_LEVEL_ALL 4

/*
  Set the camera resolution
*/
// #define FRAMESIZE FRAMESIZE_QVGA // 320x240
#define FRAMESIZE FRAMESIZE_VGA // 640x480
// #define FRAMESIZE FRAMESIZE_SVGA // 800x600
// #define FRAMESIZE FRAMESIZE_XGA // 1024x768
// #define FRAMESIZE FRAMESIZE_HD // 1280x720

/*
  Set the camera flip
*/
#define CAMERA_VERTICAL_FLIP 1
#define CAMERA_HORIZONTAL_FLIP 1

/*
  Set the SERIAL_TIMEOUT (ms)
*/
#define SERIAL_TIMEOUT 100


String WIFI_MODES[3] = {"None", "STA", "AP"};
DynamicJsonDocument sendBuffer(WS_BUFFER_SIZE);
WS_Server ws_server = WS_Server();
WiFiHelper wifi = WiFiHelper();
bool ws_server_available = false;
String rxBuf = "";
static QueueHandle_t xQueueHttpFrame = NULL;
bool is_camera_started = false;

// Functions
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
void camera_init();
String serialRead();
void handleSet(String cmd);
void start();
void handleData(String data);
void led_status_handle();
void debug(String msg);
void info(String msg);
void error(String msg);
void debug(String msg, String data);
void info(String msg, String data);
void error(String msg, String data);


void setup() {
  Serial.begin(115200);
  Serial.setTimeout(SERIAL_TIMEOUT);

  Serial.println(F("[Init]"));

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  led_init(CAMERA_PIN_LED); //init status_led 
  LED_STATUS_DISCONNECTED(); //turn on status_led
  pinMode(CAMERA_PIN_FLASH, OUTPUT); // init flash lamp
  digitalWrite(CAMERA_PIN_FLASH, 0); // 0:turn off flash lamp

  // start();

  // Serial.println(F("[OK]"));
  info(F("Init...OK"));

  // log_i("psram: %d", psramFound());
  // log_i("Total heap: %d", ESP.getHeapSize()); 
  // log_i("Free heap: %d", ESP.getFreeHeap());
  // log_i("Total PSRAM: %d", ESP.getPsramSize());
  // log_i("Free PSRAM: %d", ESP.getFreePsram());

}

// websocket loop && camera init
void ws_server_camera_handler() {
  if (wifi.is_connected) {
    ws_server.loop();
    if (!is_camera_started){
      camera_init();
    }
  } 
} 

void serial_received_handler() {
  // serial receive
  rxBuf = serialRead();
  if (rxBuf.length() > 0) {
    debug("RX Receive: ", rxBuf);
    // delay(10);
    if (rxBuf.substring(0, 4) == "SET+") {
      handleSet(rxBuf.substring(4));
    } else if (rxBuf.substring(0, 3) == "WS+") {
      String out = rxBuf.substring(3);
      handleData(out);
    }
  }
}


void loop() {

  led_status_handler();
  ws_server_camera_handler();
  serial_received_handler();

}

void camera_init(){
  // Serial.print(F("camera init by mode:"));Serial.println("steam");
  
  xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  pixformat_t pixel_format = PIXFORMAT_JPEG;
  register_camera(
      pixel_format, FRAMESIZE, 1, xQueueHttpFrame, CAMERA_VERTICAL_FLIP, CAMERA_HORIZONTAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  register_httpd(xQueueHttpFrame, NULL, true);
  is_camera_started = true;

  info("camera stream start on: ", videoUrl);
  // log_i("Free PSRAM: %d", ESP.getFreePsram());
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
  if (cmd.substring(0, 4) == "NAME"){
    name = cmd.substring(4);
    debug("Set NAME: ", name);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "TYPE"){
    type = cmd.substring(4);
    debug("Set TYPE: ", type);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "SSID"){
    ssid = cmd.substring(4);
    debug("Set SSID: ", ssid);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 3) == "PSK"){
    password = cmd.substring(3);
    debug("Set password: ", password);
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "PORT"){
    port = cmd.substring(4).toInt();
    debug("Set port: ", String(port));
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
  } else {
    error("Unknown command");
  }
}

// test: WS+3;5;10,30;forward;0;1;4;-100;40.4;50,60;-45.5;backward;50564,33630,333
void handleData(String data) {
  clearSendBuffer();
  debug("Data: ", data);
  for (int i = 0; i < REGIONS_LENGTH; i++) {
    String str = getStrOf(data, i, ';');
    String region = String(REGIONS[i]);
    if (str.indexOf(',') != -1) {
      JsonArray arr = sendBuffer.createNestedArray(region);
      uint8_t j =0;
      while (1) {
        String value = getStrOf(str, j, ',');
        if (value.length() == 0) {
          break;
        }
        if (value.indexOf('.') != -1) {
          arr.add(value.toFloat());
        } else {
          arr.add(value.toInt());
        }
        j++;
      }
    } else if (str.indexOf('.') != -1) {
      sendBuffer[region] = str.toFloat();
    } else if (str[0] > '0' && str[0] < '9' || str[0] == '-') {
      sendBuffer[region] = str.toInt();
    } else if (str == "true") {
      sendBuffer[region] = true;
    } else if (str == "false") {
      sendBuffer[region] = false;
    } else {
      sendBuffer[region] = str;
    }
  }
  char payload[WS_BUFFER_SIZE];
  serializeJson(sendBuffer, payload);
  debug("Payload: ", payload);
  ws_server.send(String(payload));
}

void clearSendBuffer() {
  sendBuffer.clear();
  sendBuffer["Name"] = name;
  sendBuffer["Type"] = type;
  sendBuffer["Check"] = CHECK_TEXT;
  sendBuffer["video"] = videoUrl;
}

String getStrOf(String str, uint8_t index, char divider) {
  uint8_t start, end;
  uint8_t length = str.length();
  uint8_t i, j;
  // Get start index
  if (index == 0) {
    start = 0;
  } else {
    for (start = 0, j = 1; start < length; start++) {
      if (str[start] == divider) {
        if (index == j) {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++) {
    // Serial.println((int)str[end]);
    if (str[end] == divider) {
      break;
    }
  }
  // Copy result
  return str.substring(start, end);
}

void setStrOf(char* str, uint8_t index, String value) {
  uint8_t start, end;
  uint8_t length = strlen(str);
  uint8_t i, j;
  // Get start index
  if (index == 0) {
    start = 0;
  } else {
    for (start = 0, j = 1; start < length; start++) {
      if (str[start] == ';') {
        if (index == j) {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++) {
    if (str[end] == ';') {
      break;
    }
  }
  String strValue = String(str).substring(0, start) + value + String(str).substring(end);
  strcpy(str, strValue.c_str());
}

void start() {
  
  LED_STATUS_ERROOR();
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
      LED_STATUS_ERROOR();
    } else {
      LED_STATUS_DISCONNECTED();
      if (ws_server_available == false) {
        // ws_server.begin(port);
        ws_server.begin(port, name, type, CHECK_TEXT);
        ws_server_available = true;
        debug("Websocket on!");
      } else {
        debug("Websocket already on!");
      } 
      Serial.print("[OK] ");Serial.println(wifi.ip);
      videoUrl = String("http://") + wifi.ip + ":9000/mjpg";
      
      clearSendBuffer();
    }
  }
}


void debug(String msg) {
  #if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_DEBUG) 
    Serial.print(F("[CAM_D] "));
    Serial.println(msg);
  #endif
}

void info(String msg) {
  #if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_INFO) 
    Serial.print(F("[CAM_I] "));
    Serial.println(msg);
  #endif
}

void error(String msg) {
  #if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_ERROR) 
    Serial.print(F("[CAM_E] "));
    Serial.println(msg);
  #endif
}

void debug(String msg, String data) {
  #if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_DEBUG) 
    Serial.print(F("[CAM_D] "));
    Serial.print(msg);
    Serial.println(data);
  #endif
}

void info(String msg, String data) {
  #if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_INFO) 
    Serial.print(F("[CAM_I] "));
    Serial.print(msg);
    Serial.println(data);
  #endif
}

void error(String msg, String data) {
  #if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_ERROR) 
    Serial.print(F("[CAM_E] "));
    Serial.print(msg);
    Serial.println(data);
  #endif
}














