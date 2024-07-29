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
#define VERSION "1.5.2"

#include "led_status.hpp"
#include "who_camera.h"
#include "camera_server.hpp"
#include "ws_server.h"
#include "wifi_helper.h"
#include "ArduinoJson.h"
#include "soc/soc.h" // disable brownout detector
#include "soc/rtc_cntl_reg.h"
#include "esp32/rom/rtc.h" // rst reason
#include <Preferences.h>   // Save configs
#include <ESPmDNS.h>       // mDNS

#include "ota_server.hpp" // OTA
// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ResetReason/ResetReason.ino

/* Select development board */
#define ESP32_CAM
#include "pins.h" // after define development board

/* ----------------------- Configuration -------------------------------- */
int port = 8765;

String name;
String type;
String apSsid;
String apPassword;
String staSsid;
String staPassword;

/* Set check info for Sunfounder Controller APP */
extern String videoUrl;
extern String videoTemplate;
#define CHECK_TEXT "SC"

/* Set the Debug Level */
#define DEBUG_LEVEL CAM_DEBUG_LEVEL_DEBUG
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
#define SERIAL_TIMEOUT 100 // timeout 100ms
#define CHAR_TIMEOUT 5     // char timeout (ms)

/* Set default AP SSID, AP Password, Name, Type*/
#define DEFAULT_AP_SSID "AI Camera"
#define DEFAULT_AP_PASSWORD "12345678"
#define DEFAULT_NAME "AI Camera"
#define DEFAULT_TYPE "AI Camera"

/* ----------------------- Global Variables -------------------------- */
WiFiHelper wifi = WiFiHelper();
Preferences prefs;

WS_Server wsServer = WS_Server();

static QueueHandle_t xQueueHttpFrame = NULL;
bool isCameraStarted = false;
bool inited = false; // For default config settings. All settings send befor inited will be treated as default settings.
                     // default settings will be ignored if settings in flash are not empty.

String rxBuf = "";

bool otaStarted = false;

/* ----------------------- Functions -------------------------------- */
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
void cameraInit();
String serialRead();
void readConfig();
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
void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(SERIAL_TIMEOUT);
  delay(2);

  int reason = rtc_get_reset_reason(0); // cpu0
  if (reason != 12)
  { // 12, SW_CPU_RESET,  Software reset CPU
    Serial.println(VERSION);
  }
  else
  {
    Serial.print(F("[OK] ")); // send [OK] when Software Reset
    Serial.println(VERSION);
  }

  videoTemplate = "http://ip:9000/mjpg";

  Serial.println(F("[Init]"));
  wifi.begin();
  prefs.begin("config");
  readConfig();

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  led_init(CAMERA_PIN_LED);          // init status_led
  LED_STATUS_DISCONNECTED();         // turn on status_led
  pinMode(CAMERA_PIN_FLASH, OUTPUT); // init flash lamp
  digitalWrite(CAMERA_PIN_FLASH, 0); // 0:turn off flash lamp

  wifi.connectAp(apSsid, apPassword);
  mDNSInit();
  otaBegin(VERSION);

  // Check if factory reset needed
  factoryResetCheck();

  // log_i("psram: %d", psramFound());
  // log_i("Total heap: %d", ESP.getHeapSize());
  // log_i("Free heap: %d", ESP.getFreeHeap());
  // log_i("Total PSRAM: %d", ESP.getPsramSize());
  // log_i("Free PSRAM: %d", ESP.getFreePsram());
}

void loop()
{
  ledStatusHandler();
  wsServerCameraHandler();
  serialReceivedHandler();
  otaHandler();
  delay(6);
}

/*--------------------- Functions------------------------------*/
/* websocket loop && camera init */
void wsServerCameraHandler()
{
  if (wifi.isConnected)
  {
    if (wifi.staConnected)
    {
      wifi.checkSta();
    }
    wsServer.loop();
    if (!isCameraStarted)
    {
      cameraInit();
    }
  }
}

void otaHandler()
{
  if (otaStarted)
    otaLoop();
}

void serialReceivedHandler()
{
  rxBuf = serialRead();
  if (rxBuf.length() > 0)
  {
    // debug("RX: ", rxBuf);
    if (rxBuf.substring(0, 4) == "SET+")
    {
      handleSet(rxBuf.substring(4));
    }
    else if (rxBuf.substring(0, 3) == "WS+")
    {
      String out = rxBuf.substring(3);
      wsServer.send(out);
    }
    else if (rxBuf.substring(0, 4) == "WSB+")
    {
      String _data = rxBuf.substring(4);
      size_t len = _data.length();
      uint8_t *byte_data = (uint8_t *)(_data.c_str());
      wsServer.sendBIN(byte_data, len);
    }
  }
}

String serialRead()
{
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
  while (Serial.available() && millis() - charTime < CHAR_TIMEOUT)
  {
    // ------------ read data --------------------
    inChar = (char)Serial.read();

    if (isValid == false)
    { // check & discard the 0xff at the beginning
      if ((int)inChar == 0xff || inChar == '\r' || inChar == '\n')
      {
        len = 0;
        continue;
      }
      else
      {
        isValid = true;
      }
    }

    if ((int)inChar == StartCode)
    {
      isBin = true;
    }

    // ------------ text data --------------------

    if (!isBin)
    {
      if (inChar == '\n')
      { // \r\n receive end
        break;
      }
      else if (inChar == '\r')
      {
        charTime = millis();
        continue;
      }
      else
      {
        buf += inChar;
        charTime = millis();
      }
    }

    // ------------ binary data --------------------
    else if (isBin)
    {
      len++;
      // get binary data length in 2nd byte
      if (len == 2)
        dataLen = (uint8_t)inChar;

      if (len <= dataLen)
      {
        buf += inChar;
        charTime = millis();
      }

      if (len >= dataLen)
      {
        break;
      }
    }

  } // while

  // debug(buf);
  return buf;
}

void readConfig()
{
  name = prefs.getString("name", DEFAULT_NAME);
  type = prefs.getString("type", DEFAULT_TYPE);
  apSsid = prefs.getString("apSsid", DEFAULT_AP_SSID);
  apPassword = prefs.getString("apPassword", DEFAULT_AP_PASSWORD);
  staSsid = prefs.getString("staSsid", "");
  staPassword = prefs.getString("staPassword", "");
  debug("readConfig");
  debug("name: ", name);
  debug("type: ", type);
  debug("apSsid: ", apSsid);
  debug("apPassword: ", apPassword);
  debug("staSsid: ", staSsid);
  debug("staPassword: ", staPassword);
}

void cameraInit()
{
  xQueueHttpFrame = xQueueCreate(2, 2 * sizeof(camera_fb_t *));
  pixformat_t pixel_format = PIXFORMAT_JPEG;
  register_camera(
      pixel_format, FRAMESIZE, FB_COUNT, xQueueHttpFrame, CAMERA_VERTICAL_FLIP, CAMERA_HORIZONTAL_FLIP,
      CAMERA_PIN_Y2, CAMERA_PIN_Y3, CAMERA_PIN_Y4, CAMERA_PIN_Y5, CAMERA_PIN_Y6,
      CAMERA_PIN_Y7, CAMERA_PIN_Y8, CAMERA_PIN_Y9, CAMERA_PIN_XCLK,
      CAMERA_PIN_PCLK, CAMERA_PIN_VSYNC, CAMERA_PIN_HREF, CAMERA_PIN_SIOD,
      CAMERA_PIN_SIOC, CAMERA_PIN_PWDN, CAMERA_PIN_RESET);
  register_httpd(xQueueHttpFrame, NULL, true);
  isCameraStarted = true;
  log_i("Free PSRAM: %d", ESP.getFreePsram());
  info("camera stream start on: ", videoUrl);
}

void mDNSInit()
{
  if (MDNS.begin(name))
  { // Set mDNS
    otaStarted = true;
  }
  else
  {
    otaStarted = false;
    Serial.println(F("Warning: Error setting up MDNS responder!"));
  }
}

void handleSet(String cmd)
{
  String temp;
  // ------------ 3 characters command  ------------
  String _3_chars_cmd = cmd.substring(0, 3);
  // PSK
  if (_3_chars_cmd == "PSK")
  {
    temp = cmd.substring(3);
    if (apPassword.compareTo(DEFAULT_AP_PASSWORD) == 0 || inited == true)
    {
      apPassword = temp;
      debug("Set AP password: ", apPassword);
      prefs.putString("apPassword", apPassword);
      Serial.println("[OK] PSK is deprecating, use APPSK or STAPSK");
    }
    else
    {
      Serial.println("[OK] AP already set");
    }
    return;
  }

  // ------------ 4 characters command  ------------
  String _4_chars_cmd = cmd.substring(0, 4);
  // NAME
  if (_4_chars_cmd == "NAME")
  {
    temp = cmd.substring(4);
    if (name.compareTo(DEFAULT_NAME) == 0 || inited == true)
    {
      name = temp;
      debug("Set NAME: ", name);
      prefs.putString("name", name);
      Serial.println("[OK]");
    }
    else
    {
      Serial.println("[OK] NAME already set");
    }
    return;
  }
  // TYPE
  else if (_4_chars_cmd == "TYPE")
  {
    type = cmd.substring(4);
    debug("Set TYPE: ", type);
    prefs.putString("type", type);
    Serial.println("[OK]");
    return;
  }
  // SSID
  else if (_4_chars_cmd == "SSID")
  {
    temp = cmd.substring(4);
    if (staSsid.compareTo("") == 0 || inited == true)
    {
      staSsid = temp;
      debug("Set SSID: ", staSsid);
      prefs.putString("staSsid", staSsid);
      Serial.println("[OK] SSID is deprecating, use APSSID or STASSID");
    }
    else
    {
      Serial.println("[OK] SSID already set");
    }
    return;
  }
  // PORT
  else if (_4_chars_cmd == "PORT")
  {
    port = cmd.substring(4).toInt();
    debug("Set port: ", String(port));
    Serial.println("[OK]");
    return;
  }
  // MODE
  else if (_4_chars_cmd == "MODE")
  {
    Serial.println("[OK] MODE is deprecated");
    return;
  }
  // LAMP
  else if (_4_chars_cmd == "LAMP")
  {
    uint8_t brightness_level = cmd.substring(4).toInt();
    // digitalWrite(CAMERA_PIN_FLASH, lamp_sw); // 0:turn off flash lamp, 1:turn on flash lamp
    if (brightness_level > 10)
      brightness_level = 10;
    analogWrite(CAMERA_PIN_FLASH, brightness_level * 25); //  brightness_level: 0~10, to pwm 0 ~ 250
    Serial.println("[OK]");
    return;
  }

  // ------------ 5 characters command  ------------
  String _5_chars_cmd = cmd.substring(0, 5);
  // RESET
  if (_5_chars_cmd == "RESET")
  {
    debug("Reset");
    delay(10);
    ESP.restart();
    return;
  }
  // START
  else if (_5_chars_cmd == "START")
  {
    start();
    return;
  }
  // AP PSK
  else if (_5_chars_cmd == "APPSK")
  {
    temp = cmd.substring(5);
    if (apPassword.compareTo(DEFAULT_AP_PASSWORD) == 0 || inited == true)
    {
      apPassword = temp;
      debug("Set AP password: ", apPassword);
      prefs.putString("apPassword", apPassword);
      Serial.println("[OK]");
    }
    else
    {
      Serial.println("[OK] AP already set");
    }
    return;
  }

  // ------------ 6 characters command  ------------
  String _6_chars_cmd = cmd.substring(0, 6);
  // AP SSID
  if (_6_chars_cmd == "APSSID")
  {
    temp = cmd.substring(6);
    if (apSsid.compareTo(DEFAULT_AP_SSID) == 0 || inited == true)
    {
      apSsid = temp;
      debug("Set AP ssid: ", apSsid);
      debug("AP ssid temp: ", temp);
      prefs.putString("apSsid", apSsid);
      Serial.println("[OK]");
    }
    else
    {
      debug("AP already set");
      Serial.println("[OK] AP already set");
    }
    return;
  }
  // STA PSK
  else if (_6_chars_cmd == "STAPSK")
  {
    temp = cmd.substring(6);
    if (staPassword.length() == 0 || inited == true)
    {
      staPassword = temp;
      debug("Set STA password: ", staPassword);
      prefs.putString("staPassword", staPassword);
      Serial.println("[OK]");
    }
    else
    {
      Serial.println("[OK] STA already set");
    }
    return;
  }
  // Restart STA
  else if (_6_chars_cmd == "RSTSTA")
  {
    if (staSsid.length() > 0 && staPassword.length() > 0)
    {
      bool result = wifi.connectSta(staSsid, staPassword);
      if (result)
      {
        debug("STA connected");
        wsServer.setStaIp(wifi.staIp);
        Serial.print("[OK]");
        Serial.println(wifi.staIp);
      }
      else
      {
        debug("STA connect failed");
        Serial.println("[ERROR] STA connect failed");
      }
    }
    else
    {
      debug("STA SSID or password is empty");
      Serial.println("[ERROR] STA not set");
    }
    return;
  }
  // Reset Config
  else if (_6_chars_cmd == "RSTCFG")
  {
    prefs.clear();
    readConfig();
    debug("Reset config");
    Serial.println("[OK]");
    return;
  }
  // ------------ 7 characters command  ------------
  String _7_chars_cmd = cmd.substring(0, 7);
  // SSID
  if (_7_chars_cmd == "STASSID")
  {
    temp = cmd.substring(7);
    if (staSsid.length() == 0 || inited == true)
    {
      staSsid = temp;
      debug("Set STA SSID: ", staSsid);
      prefs.putString("staSsid", staSsid);
      Serial.println("[OK]");
    }
    else
    {
      Serial.println("[OK] STA SSID already set");
    }
    return;
  }

  // ----------- if no retrun before -----------
  Serial.println("[ERROR] SET+ Unknown command");
}

String getStrOf(String str, uint8_t index, char divider)
{
  uint8_t start, end;
  uint8_t length = str.length();
  uint8_t i, j;
  // Get start index
  if (index == 0)
  {
    start = 0;
  }
  else
  {
    for (start = 0, j = 1; start < length; start++)
    {
      if (str[start] == divider)
      {
        if (index == j)
        {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++)
  {
    if (str[end] == divider)
    {
      break;
    }
  }
  // Copy result
  return str.substring(start, end);
}

void setStrOf(char *str, uint8_t index, String value)
{
  uint8_t start, end;
  uint8_t length = strlen(str);
  uint8_t i, j;
  // Get start index
  if (index == 0)
  {
    start = 0;
  }
  else
  {
    for (start = 0, j = 1; start < length; start++)
    {
      if (str[start] == ';')
      {
        if (index == j)
        {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++)
  {
    if (str[end] == ';')
    {
      break;
    }
  }
  String strValue = String(str).substring(0, start) + value + String(str).substring(end);
  strcpy(str, strValue.c_str());
}

void start()
{
  bool staConnected = false;
  LED_STATUS_ERROR();
  if (staSsid.length() > 0 || staPassword.length() > 8)
  {
    staConnected = wifi.connectSta(staSsid, staPassword);
    if (staConnected)
    {
      debug(F("STA connected"));
    }
    else
    {
      debug(F("STA connect failed"));
      // Serial.println("[ERROR] STA connect failed");
    }
  }
  else
  {
    debug(F("STA SSID or STA password empty!"));
  }
  LED_STATUS_DISCONNECTED();
  wsServer.close();
  wsServer.begin(port, name, type, CHECK_TEXT);
  debug(F("Websocket on!"));
  Serial.print(F("[OK] "));
  if (staConnected)
  {
    wsServer.setStaIp(wifi.staIp);
    videoUrl = String("http://") + wifi.staIp + ":9000/mjpg";
    Serial.println(wifi.staIp);
  }
  else
  {
    videoUrl = String("http://") + wifi.apIp + ":9000/mjpg";
    Serial.println(wifi.apIp);
  }
  inited = true;
}

void debug(String msg)
{
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_DEBUG)
  Serial.print(F("[CAM_D] "));
  Serial.println(msg);
#endif
}

void info(String msg)
{
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_INFO)
  Serial.print(F("[CAM_I] "));
  Serial.println(msg);
#endif
}

void error(String msg)
{
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_ERROR)
  Serial.print(F("[CAM_E] "));
  Serial.println(msg);
#endif
}

void debug(String msg, String data)
{
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_DEBUG)
  Serial.print(F("[CAM_D] "));
  Serial.print(msg);
  Serial.println(data);
#endif
}

void info(String msg, String data)
{
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_INFO)
  Serial.print(F("[CAM_I] "));
  Serial.print(msg);
  Serial.println(data);
#endif
}

void error(String msg, String data)
{
#if (DEBUG_LEVEL >= CAM_DEBUG_LEVEL_ERROR)
  Serial.print(F("[CAM_E] "));
  Serial.print(msg);
  Serial.println(data);
#endif
}

void factoryResetCheck()
{
  uint8_t sense = 13;
  uint8_t pull = 15;
  pinMode(sense, INPUT_PULLUP);
  pinMode(pull, OUTPUT);
  digitalWrite(pull, 0);
  if (digitalRead(sense) == 0)
  {
    for (uint8_t i = 0; i < 2; i++)
    {
      analogWrite(CAMERA_PIN_FLASH, 1);
      delay(100);
      analogWrite(CAMERA_PIN_FLASH, 0);
      delay(100);
    }
    prefs.clear();
    while (1)
      ;
    ;
  }
  pinMode(pull, INPUT);
}
