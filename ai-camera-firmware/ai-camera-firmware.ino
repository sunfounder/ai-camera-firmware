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
    - WebSockets (by Markus Sattler) (Links2004)

  Repositorie:
    -- https://github.com/sunfounder/ai-camera-firmware

  Author: Sunfounder
  Website: http://www.sunfounder.com
           https://docs.sunfounder.com
 *******************************************************************/
#define VERSION "1.5.3.20"

#include "settings.h"
#include "camera_server.h"
#include "rom/rtc.h" // rst reason
#include "led_status.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h" // disable brownout detector
#include "camera.h"
#include "wifi_helper.h"
#include "ws_server.h"

#include "defaults.h" // Default settings
#include "log.h"
// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ResetReason/ResetReason.ino

/* Select development board */
#define ESP32_CAM
// #define ESP32_S3_CAM
#include "pins.h" // after define development board

/* ----------------------- Configuration -------------------------------- */
int port = 8765;

/* Set check info for Sunfounder Controller APP */
extern String videoUrl;
extern String videoTemplate;
#define CHECK_TEXT "SC"

/* ----------------------- Global Variables -------------------------- */
WS_Server wsServer = WS_Server();

static QueueHandle_t xQueueHttpFrame = NULL;
bool isCameraStarted = false;
bool inited =
    false; // For default config settings. All settings send befor inited will
           // be treated as default settings. default settings will be ignored
           // if settings in flash are not empty.

String rxBuf = "";

/* ----------------------- Functions -------------------------------- */
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
void cameraInit();
String serialRead();
void handleSet(String cmd);
void start();
void handleData(String data);

/*--------------------- setup() & loop() ------------------------------*/
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(SERIAL_TIMEOUT);
  delay(2);

  int reason = rtc_get_reset_reason(0); // cpu0
  if (reason != 12) {                   // 12, SW_CPU_RESET,  Software reset CPU
    Serial.println(VERSION);
  } else { // send [OK] when Software Reset
    Serial.print(F("[OK] "));
    Serial.println(VERSION);
  }

  videoTemplate = "http://ip:9000/mjpg";

  Serial.println(F("[Init]"));
  log_i("Total heap: %d", ESP.getHeapSize());
  log_i("Free heap: %d", ESP.getFreeHeap());
  log_i("Total PSRAM: %d", ESP.getPsramSize());
  log_i("Free PSRAM: %d", ESP.getFreePsram());
  wifiBegin();
  settingsBegin(VERSION);
  log_i("Total heap: %d", ESP.getHeapSize());
  log_i("Free heap: %d", ESP.getFreeHeap());
  log_i("Total PSRAM: %d", ESP.getPsramSize());
  log_i("Free PSRAM: %d", ESP.getFreePsram());

  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  ledBegin(CAMERA_PIN_LED);          // init status_led
  LED_STATUS_DISCONNECTED();         // turn on status_led
  pinMode(CAMERA_PIN_FLASH, OUTPUT); // init flash lamp
  digitalWrite(CAMERA_PIN_FLASH, 0); // 0:turn off flash lamp

  log_i("Total heap: %d", ESP.getHeapSize());
  log_i("Free heap: %d", ESP.getFreeHeap());
  log_i("Total PSRAM: %d", ESP.getPsramSize());
  log_i("Free PSRAM: %d", ESP.getFreePsram());

  wifiConnectAp(settingsGetApSsid(), settingsGetApPassword(),
                settingsGetApChannel());
  wifiSetHostname(settingsGetName());

  // Check if factory reset needed
  factoryResetCheck();

  log_i("Total heap: %d", ESP.getHeapSize());
  log_i("Free heap: %d", ESP.getFreeHeap());
  log_i("Total PSRAM: %d", ESP.getPsramSize());
  log_i("Free PSRAM: %d", ESP.getFreePsram());
  // log_i("psram: %d", psramFound());
  // log_i("Total heap: %d", ESP.getHeapSize());
  // log_i("Free heap: %d", ESP.getFreeHeap());
  // log_i("Total PSRAM: %d", ESP.getPsramSize());
  // log_i("Free PSRAM: %d", ESP.getFreePsram());
}

void loop() {
  ledStatusHandler();
  wsServerCameraHandler();
  serialReceivedHandler();
  settingsLoop();
  delay(6);
}

/*--------------------- Functions------------------------------*/
/* websocket loop && camera init */
void wsServerCameraHandler() {
  if (wifiIsConnected) {
    if (wifiIsStaConnected()) {
      wifiCheckSta();
    }
    wsServer.loop();
    if (!isCameraStarted) {
      cameraInit();
      log_i("Total heap: %d", ESP.getHeapSize());
      log_i("Free heap: %d", ESP.getFreeHeap());
      log_i("Total PSRAM: %d", ESP.getPsramSize());
      log_i("Free PSRAM: %d", ESP.getFreePsram());
    }
  }
}

void serialReceivedHandler() {
  rxBuf = serialRead();
  if (rxBuf.length() > 0) {
    // debug("RX: ", rxBuf);
    if (rxBuf.substring(0, 4) == "SET+") {
      handleSet(rxBuf.substring(4));
    } else if (rxBuf.substring(0, 3) == "WS+") {
      String out = rxBuf.substring(3);
      wsServer.send(out);
    } else if (rxBuf.substring(0, 4) == "WSB+") {
      String _data = rxBuf.substring(4);
      size_t len = _data.length();
      uint8_t *byte_data = (uint8_t *)(_data.c_str());
      wsServer.sendBIN(byte_data, len);
    }
  }
}

String serialRead() {
  String buf = "";
  char inChar;
  bool isValid = false;
  // -- binary data protocol --
  bool isBin = false;
  const uint8_t StartCode = 0x0C;
  const uint8_t EndCode = 0x0D;
  uint8_t len = 0;
  uint8_t dataLen = 3; // note that default value  need to larger than 3

  uint32_t charTime = millis();
  while (Serial.available() && millis() - charTime < CHAR_TIMEOUT) {
    // ------------ read data --------------------
    inChar = (char)Serial.read();

    if (isValid == false) { // check & discard the 0xff at the beginning
      if ((int)inChar == 0xff || inChar == '\r' || inChar == '\n') {
        len = 0;
        continue;
      } else {
        isValid = true;
      }
    }

    if ((int)inChar == StartCode) {
      isBin = true;
    }

    // ------------ text data --------------------

    if (!isBin) {
      if (inChar == '\n') { // \r\n receive end
        break;
      } else if (inChar == '\r') {
        charTime = millis();
        continue;
      } else {
        buf += inChar;
        charTime = millis();
      }
    }

    // ------------ binary data --------------------
    else if (isBin) {
      len++;
      // get binary data length in 2nd byte
      if (len == 2)
        dataLen = (uint8_t)inChar;

      if (len <= dataLen) {
        buf += inChar;
        charTime = millis();
      }

      if (len >= dataLen) {
        break;
      }
    }

  } // while

  // debug(buf);
  return buf;
}

void cameraInit() {
  xQueueHttpFrame = xQueueCreate(2, 2 * sizeof(camera_fb_t *));
  pixformat_t pixel_format = PIXFORMAT_JPEG;
  register_camera(
      pixel_format, FRAMESIZE, FB_COUNT, xQueueHttpFrame,
      settingsGetCameraVerticalFlip(), settingsGetCameraHorizontalMirror(),
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  register_httpd(xQueueHttpFrame, NULL, true);
  isCameraStarted = true;
  log_i("Free PSRAM: %d", ESP.getFreePsram());
  info("camera stream start on: ", videoUrl);
}

void handleSet(String cmd) {
  String temp;
  // ------------ 3 characters command  ------------
  String _3_chars_cmd = cmd.substring(0, 3);
  // PSK
  if (_3_chars_cmd == "PSK") {
    temp = cmd.substring(3);
    if (!settingsApPasswordChanged() || inited == true) {
      settingsSetApPassword(temp);
      debug("Set AP password: ", temp);
      Serial.println("[OK] PSK is deprecating, use APPSK or STAPSK");
    } else {
      Serial.println("[OK] AP already set");
    }
    return;
  }

  // ------------ 4 characters command  ------------
  String _4_chars_cmd = cmd.substring(0, 4);
  // NAME
  if (_4_chars_cmd == "NAME") {
    temp = cmd.substring(4);
    if (!settingsNameChanged() || inited == true) {
      settingsSetName(temp);
      debug("Set NAME: ", temp);
      Serial.println("[OK]");
    } else {
      Serial.println("[OK] NAME already set");
    }
    return;
  }
  // TYPE
  else if (_4_chars_cmd == "TYPE") {
    temp = cmd.substring(4);
    settingsSetType(temp);
    debug("Set TYPE: ", temp);
    Serial.println("[OK]");
    return;
  }
  // SSID
  else if (_4_chars_cmd == "SSID") {
    temp = cmd.substring(4);
    if (!settingsApSsidChanged() || inited == true) {
      settingsSetApSsid(temp);
      debug("Set SSID: ", temp);
      Serial.println("[OK] SSID is deprecating, use APSSID or STASSID");
    } else {
      Serial.println("[OK] SSID already set");
    }
    return;
  }
  // PORT
  else if (_4_chars_cmd == "PORT") {
    port = cmd.substring(4).toInt();
    debug("Set port: ", String(port));
    Serial.println("[OK]");
    return;
  }
  // MODE
  else if (_4_chars_cmd == "MODE") {
    Serial.println("[OK] MODE is deprecated");
    return;
  }
  // LAMP
  else if (_4_chars_cmd == "LAMP") {
    uint8_t brightness_level = cmd.substring(4).toInt();
    // digitalWrite(CAMERA_PIN_FLASH, lamp_sw); // 0:turn off flash lamp, 1:turn
    // on flash lamp
    if (brightness_level > 10)
      brightness_level = 10;
    analogWrite(CAMERA_PIN_FLASH,
                brightness_level *
                    25); //  brightness_level: 0~10, to pwm 0 ~ 250
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
  // AP PSK
  else if (_5_chars_cmd == "APPSK") {
    temp = cmd.substring(5);
    if (!settingsApPasswordChanged() || inited == true) {
      settingsSetApPassword(temp);
      debug("Set AP password: ", temp);
      Serial.println("[OK]");
    } else {
      Serial.println("[OK] AP already set");
    }
    return;
  }
  // AP Channel
  else if (_5_chars_cmd == "APCHN") {
    temp = cmd.substring(5);
    int channel = temp.toInt();
    if (channel < 1 || channel > 13) {
      Serial.println("[ERROR] AP channel out of range");
      return;
    }
    settingsSetApChannel(channel);
    debug("Set AP channel: ", String(channel));
    Serial.println("[OK]");
  }

  // ------------ 6 characters command  ------------
  String _6_chars_cmd = cmd.substring(0, 6);
  // AP SSID
  if (_6_chars_cmd == "APSSID") {
    temp = cmd.substring(6);
    if (!settingsApSsidChanged() || inited == true) {
      settingsSetApSsid(temp);
      debug("Set AP ssid: ", temp);
      Serial.println("[OK]");
    } else {
      debug("AP already set");
      Serial.println("[OK] AP already set");
    }
    return;
  }
  // STA PSK
  else if (_6_chars_cmd == "STAPSK") {
    temp = cmd.substring(6);
    if (!settingsStaPasswordChanged() || inited == true) {
      settingsSetStaPassword(temp);
      debug("Set STA password: ", temp);
      Serial.println("[OK]");
    } else {
      Serial.println("[OK] STA already set");
    }
    return;
  }
  // Restart STA
  else if (_6_chars_cmd == "RSTSTA") {
    String staSsid = settingsGetStaSsid();
    String staPassword = settingsGetStaPassword();
    if (staSsid.length() > 0 && staPassword.length() > 0) {
      bool result = wifiConnectSta(staSsid, staPassword);
      if (result) {
        debug("STA connected");
        Serial.print("[OK]");
        Serial.println(wifiGetStaIp());
      } else {
        debug("STA connect failed");
        Serial.println("[ERROR] STA connect failed");
      }
    } else {
      debug("STA SSID or password is empty");
      Serial.println("[ERROR] STA not set");
    }
    return;
  }
  // Reset Config
  else if (_6_chars_cmd == "RSTCFG") {
    settingsReset();
    debug("Reset config");
    Serial.println("[OK]");
    return;
  }
  // ------------ 7 characters command  ------------
  String _7_chars_cmd = cmd.substring(0, 7);
  // SSID
  if (_7_chars_cmd == "STASSID") {
    if (!settingsStaSsidChanged() || inited == true) {
      settingsSetStaSsid(temp);
      debug("Set STA SSID: ", temp);
      Serial.println("[OK]");
    } else {
      Serial.println("[OK] STA SSID already set");
    }
    return;
  }

  // ----------- if no retrun before -----------
  Serial.println("[ERROR] SET+ Unknown command");
}

void start() {
  bool staConnected = false;
  LED_STATUS_ERROR();
  String staSsid = settingsGetStaSsid();
  String staPassword = settingsGetStaPassword();
  if (staSsid.length() > 0 || staPassword.length() > 8) {
    staConnected = wifiConnectSta(staSsid, staPassword);
    if (staConnected) {
      debug(F("STA connected"));
    } else {
      debug(F("STA connect failed"));
      // Serial.println("[ERROR] STA connect failed");
    }
  } else {
    debug(F("STA SSID or STA password empty!"));
  }
  LED_STATUS_DISCONNECTED();
  wsServer.close();
  wsServer.begin(port, settingsGetName(), settingsGetType(), CHECK_TEXT);
  debug(F("Websocket on!"));
  Serial.print(F("[OK] "));
  if (staConnected) {
    videoUrl = String("http://") + wifiGetStaIp() + ":9000/mjpg";
    Serial.println(wifiGetStaIp());
  } else {
    videoUrl = String("http://") + wifiGetApIp() + ":9000/mjpg";
    Serial.println(wifiGetApIp());
  }
  inited = true;
}

void factoryResetCheck() {
  uint8_t sense = 13;
  uint8_t pull = 15;
  pinMode(sense, INPUT_PULLUP);
  pinMode(pull, OUTPUT);
  digitalWrite(pull, 0);
  if (digitalRead(sense) == 0) {
    for (uint8_t i = 0; i < 2; i++) {
      analogWrite(CAMERA_PIN_FLASH, 1);
      delay(100);
      analogWrite(CAMERA_PIN_FLASH, 0);
      delay(100);
    }
    settingsReset();
    while (1)
      ;
    ;
  }
  pinMode(pull, INPUT);
}
