/*******************************************************************
  Camera firmware for serial communication with the lower computer，
  and wenbsocket communication with the Sunfounder Controller APP,
  targeted at ESP32-CAM and TTGO_CAMERA.

  Development test environment:
    - Arduino IDE 2.0.3
  Board tools:
    - Arduino AVR Boards 2.0.3
    - esp32 (by Espressif Systems) 2.0.7
  Libraries:
    - ArduinoJson (by Benoit Blanchon)
    - WebSockets (by Markus Sattler)

  Version: 1.2.0
    -- https://github.com/sunfounder/ai-camera-firmware
  
  Author: Sunfounder
  Website: http://www.sunfounder.com
           https://docs.sunfounder.com
 *******************************************************************/
#define VERSION "1.2.0"

#include "led_status.hpp"
#include "who_camera.h"
#include "camera_server.hpp"
#include "ws_server.h"
#include "wifi_helper.h"
#include "ArduinoJson.h"
#include "soc/soc.h"    // disable brownout detector
#include "soc/rtc_cntl_reg.h"
#include "esp32/rom/rtc.h" // rst reason 
// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ResetReason/ResetReason.ino

/* Select development board */
#define ESP32_CAM
// #define TTGO_CAMERA
#include "pins.h" // after define development board

/* ----------------------- Configuration -------------------------------- */
/* Set Wifi mode, SSID and password */
int mode = AP;  // STA or AP
String ssid = "aiCAM";
String password = "12345678";

// int mode = STA;  // STA or AP
// String ssid = "xxxxxx";
// String password = "xxxxxxxx";

/* Set websockets port 
  Sunfounder Controller APP fixed using port 8765
*/
int port = 8765;

/* Set check info for Sunfounder Controller APP */
String name = "AI Camera";
String type = "AI_Camera";
extern String videoUrl;
#define CHECK_TEXT "SC"

/* Set the Debug Level */
#define DEBUG_LEVEL CAM_DEBUG_LEVEL_INFO
#define CAM_DEBUG_LEVEL_OFF 0
#define CAM_DEBUG_LEVEL_ERROR 1
#define CAM_DEBUG_LEVEL_INFO 2
#define CAM_DEBUG_LEVEL_DEBUG 3
#define CAM_DEBUG_LEVEL_ALL 4

/* Set the camera resolution */
// #define FRAMESIZE FRAMESIZE_QVGA // 320x240
// #define FRAMESIZE FRAMESIZE_HVGA // 480x320
#define FRAMESIZE FRAMESIZE_VGA // 640x480
// #define FRAMESIZE FRAMESIZE_SVGA // 800x600
// #define FRAMESIZE FRAMESIZE_XGA // 1024x768
// #define FRAMESIZE FRAMESIZE_HD // 1280x720

/* Set size of fb_count */
#define FB_COUNT 2

/* Set the camera flip */
#define CAMERA_VERTICAL_FLIP 1
#define CAMERA_HORIZONTAL_FLIP 1

/* Set the SERIAL_TIMEOUT (ms) */
#define SERIAL_TIMEOUT 100  // timeout 100ms
#define CHAR_TIMEOUT 5 // char timeout (ms)

/* ----------------------- Global Variables -------------------------- */
String WIFI_MODES[3] = {"None", "STA", "AP"};
WiFiHelper wifi = WiFiHelper();

WS_Server ws_server = WS_Server();

static QueueHandle_t xQueueHttpFrame = NULL;
bool is_camera_started = false;

String rxBuf = "";

/* ----------------------- Functions -------------------------------- */
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
void camera_init();
String serialRead();
void handleSet(String cmd);
void start();
void handleData(String data);
void debug(String msg);
void info(String msg);
void error(String msg);
void debug(String msg, String data);
void info(String msg, String data);
void error(String msg, String data);

/*--------------------- setup() & loop() ------------------------------*/
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(SERIAL_TIMEOUT);

  Serial.println(F("[Init]"));

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  led_init(CAMERA_PIN_LED); //init status_led 
  LED_STATUS_DISCONNECTED(); //turn on status_led
  pinMode(CAMERA_PIN_FLASH, OUTPUT); // init flash lamp
  digitalWrite(CAMERA_PIN_FLASH, 0); // 0:turn off flash lamp
  
  int reason = rtc_get_reset_reason(0); // cpu0
  if (reason != 12) { // 12, SW_CPU_RESET,  Software reset CPU
    Serial.println(VERSION);
  } else {
    Serial.print(F("[OK] ")); // send [OK] when Software Reset
    Serial.println(VERSION); 
  }

  // log_i("psram: %d", psramFound());
  // log_i("Total heap: %d", ESP.getHeapSize()); 
  // log_i("Free heap: %d", ESP.getFreeHeap());
  // log_i("Total PSRAM: %d", ESP.getPsramSize());
  // log_i("Free PSRAM: %d", ESP.getFreePsram());

}

void loop() {
  led_status_handler();
  ws_server_camera_handler();
  serial_received_handler();
  delay(6);
}

/*--------------------- Functions------------------------------*/
/* websocket loop && camera init */
void ws_server_camera_handler() {
  if (wifi.is_connected) {
    if (mode == STA) {
      wifi.check_status();
    }
    ws_server.loop();
    if (!is_camera_started){
      camera_init();
    }
  } 
} 

void serial_received_handler() {
  rxBuf = serialRead();
  if (rxBuf.length() > 0) {
    debug("RX: ", rxBuf);
    if (rxBuf.substring(0, 4) == "SET+") {
      handleSet(rxBuf.substring(4));
    } else if (rxBuf.substring(0, 3) == "WS+") {
      String out = rxBuf.substring(3);
      ws_server.send(out);
    }
  }
}

String serialRead() {
  String buf = "";
  char inChar;
  uint32_t char_time = millis();
  while (Serial.available() || millis() - char_time < CHAR_TIMEOUT) {
    inChar = (char)Serial.read();
    if (inChar == '\n') {
      break;
    } else if (inChar == '\r') {
      char_time = millis();
      continue;
    } else if ((int)inChar != 255) {
      buf += inChar;
      char_time = millis();
    }
  }
  return buf;
}

void camera_init(){
  xQueueHttpFrame = xQueueCreate(2, 2*sizeof(camera_fb_t *));
  pixformat_t pixel_format = PIXFORMAT_JPEG;
  register_camera(
      pixel_format, FRAMESIZE, FB_COUNT, xQueueHttpFrame, CAMERA_VERTICAL_FLIP, CAMERA_HORIZONTAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  register_httpd(xQueueHttpFrame, NULL, true);
  is_camera_started = true;
  log_i("Free PSRAM: %d", ESP.getFreePsram());
  info("camera stream start on: ", videoUrl);
}

void handleSet(String cmd) {
  // ------------ 3 characters command  ------------
  String _3_chars_cmd = cmd.substring(0, 3);
  // PSK
  if (_3_chars_cmd == "PSK") {
    password = cmd.substring(3);
    debug("Set password: ", password);
    Serial.println("[OK]");
    return;
  }

  // ------------ 4 characters command  ------------
  String _4_chars_cmd = cmd.substring(0, 4);
  // SSID 
  if (_4_chars_cmd == "SSID") {
    ssid = cmd.substring(4);
    debug("Set SSID: ", ssid);
    Serial.println("[OK]");
    return;
  }
  // NAME 
  if (_4_chars_cmd == "NAME"){
    name = cmd.substring(4);
    debug("Set NAME: ", name);
    Serial.println("[OK]");
    return;
  } 
  // TYPE
  else if (_4_chars_cmd == "TYPE"){
    type = cmd.substring(4);
    debug("Set TYPE: ", type);
    Serial.println("[OK]");
    return;
  } 
  // SSID
  else if (_4_chars_cmd == "SSID"){
    ssid = cmd.substring(4);
    debug("Set SSID: ", ssid);
    Serial.println("[OK]");
    return;
  } 
  // PORT
  else if (_4_chars_cmd == "PORT"){
    port = cmd.substring(4).toInt();
    debug("Set port: ", String(port));
    Serial.println("[OK]");
    return;
  } 
  // MODE
  else if (_4_chars_cmd == "MODE"){
    mode = cmd.substring(4).toInt();
    debug("Set mode: ", WIFI_MODES[mode]);
    Serial.println("[OK]");
    return;
  }

  // ------------ 5 characters command  ------------
  String _5_chars_cmd = cmd.substring(0, 5);
  // RESET
  if (_5_chars_cmd == "RESET") {
    debug("Reset");
    delay(10);
    ESP.restart();
    return;
  }
  // START
  else if (_5_chars_cmd == "START") {
    start();
    return;
  }

  // ----------- if no retrun before -----------
  Serial.println("[ERROR] SET+ Unknown command");
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
      ws_server.close();
      ws_server.begin(port, name, type, CHECK_TEXT);
      debug("Websocket on!");
      Serial.print("[OK] ");Serial.println(wifi.ip);
      videoUrl = String("http://") + wifi.ip + ":9000/mjpg";
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
