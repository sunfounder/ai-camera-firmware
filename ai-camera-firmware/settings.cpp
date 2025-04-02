#include "settings.h"

#include <Preferences.h>
#include <WebServer.h>
#include <Update.h>
#include "esp_camera.h"

#include "defaults.h"
#include "wifi_helper.h"
#include "www/index.h"
#include "www/favicon.h"
#include "www/css.h"
#include "www/js.h"
#include "log.h"

WebServer server(80);
Preferences preferences;

String version; // xxx.xxx.xxx
String name;
String type;
String apSsid;
String apPassword;
int apChannel;
String staSsid;
String staPassword;
bool camHFlip;
bool camVFlip;
int camBrightness;
int camContrast;
int camSaturation;
int camSharpness;

#define HEADER_CONTENT_ENCODING F("Content-Encoding")
#define HEADER_CONTENT_ENCODING_GZIP F("gzip")
#define HEADER_CONNECTION F("Connection")
#define HEADER_CONNECTION_CLOSE F("close")

void setCrossOriginHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "OPTIONS, GET, POST, PUT, DELETE");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
}

void returnOk() {
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send(200, "text/plain", "OK");
}

void returnNameNotFound(String name) {
  String msg = String("Bad Request") + name + " not found";
  error(msg.c_str());
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send(400, "text/plain", msg.c_str());
}

void returnSetError(String name) {
  preferences.begin(SETTING_PREFERENCES, false);
  size_t free = preferences.freeEntries();
  preferences.end();
  String msg = String("Set \"") + name + "\" error, free entries: " + String(free);
  error(msg.c_str());
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send(502, "text/plain", msg.c_str());
}

void postStringHandler(const char *name, bool (*callback)(String)) {
  if (server.hasArg(name)) {
    bool result = callback(server.arg(name));
    if (result) {
      returnOk();
    } else {
      returnSetError(name);
    }
  } else {
    returnNameNotFound(name);
  }
}
void postIntHandler(const char *name, bool (*callback)(int)) {
  if (server.hasArg(name)) {
    int value = server.arg(name).toInt();
    bool result = callback(value);
    if (result) {
      returnOk();
    } else {
      returnSetError(name);
    }
  } else {
    returnNameNotFound(name);
  }
}
void postBoolHandler(const char *name, bool (*callback)(bool)) {
  if (server.hasArg(name)) {
    String value = server.arg(name);
    bool bValue = (value == "true" || value == "1");
    bool result = callback(bValue);
    if (result) {
      returnOk();
    } else {
      returnSetError(name);
    }
  } else {
    returnNameNotFound(name);
  }
}

void handleGetIndex() {
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONTENT_ENCODING, HEADER_CONTENT_ENCODING_GZIP);
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send_P(200, "text/html", (const char*)index_html_gz, index_html_gz_len);
}
void handleGetJs() {
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONTENT_ENCODING, HEADER_CONTENT_ENCODING_GZIP);
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send_P(200, "text/javascript", (const char*)main_6ebed670_js_gz, main_6ebed670_js_gz_len);
}
void handleGetCss() {
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONTENT_ENCODING, HEADER_CONTENT_ENCODING_GZIP);
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send_P(200, "text/css", (const char*)main_75b37e2b_css_gz, main_75b37e2b_css_gz_len);
}
void handleGetFavicon() {  
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONTENT_ENCODING, HEADER_CONTENT_ENCODING_GZIP);
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send_P(200, "image/x-icon", (const char*)favicon_ico_gz, favicon_ico_gz_len);
}
void handleGetSettings() {
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send(
    200, "application/json",
    String("{") + 
      "\"version\":\"" + version + "\"," +
      "\"name\":\"" + name + "\"," +
      "\"type\":\"" + type + "\"," +
      "\"apSsid\":\"" + apSsid + "\"," +
      "\"apPassword\":\"" + apPassword + "\"," +
      "\"apChannel\":" + String(apChannel) + "," +
      "\"staSsid\":\"" + staSsid + "\"," +
      "\"staPassword\":\"" + staPassword + "\"," +
      "\"cameraHorizontalMirror\":" + (camHFlip ? "true" : "false") + "," +
      "\"cameraVerticalFlip\":" + (camVFlip ? "true" : "false") + "," +
      "\"cameraBrightness\":" + String(camBrightness) + "," +
      "\"cameraContrast\":" + String(camContrast) + "," +
      "\"cameraSaturation\":" + String(camSaturation) + "," +
      "\"cameraSharpness\":" + String(camSharpness) + "," +
      "\"macAddress\":\"" + wifiGetMacAddress() + "\"," + // Mac address: xx:xx:xx:xx:xx:xx
      "\"macPrefix\":\"" + wifiGetMacPrefix() + "\"," + // Mac address prefix: xxxxxx
      "\"staConnected\":\"" + (wifiIsStaConnected() ? "true" : "false") + "\"," +
      "\"ipAddress\":\"" + wifiGetStaIp() + "\"" +
    "}"
  );
}
void handleSetName() {
  postStringHandler("name", settingsSetName);
}
void handleSetType() {
  postStringHandler("type", settingsSetType);
}
void handleSetApSsid() {
  postStringHandler("apSsid", settingsSetApSsid);
}
void handleSetApPassword() {
  postStringHandler("apPassword", settingsSetApPassword);
}
void handleSetApChannel() {
  postIntHandler("apChannel", settingsSetApChannel);
}
void handleSetCameraHorizontalMirror() {
  postBoolHandler("camHFlip", settingsSetCameraHorizontalMirror);
  sensor_t *s = esp_camera_sensor_get();
  s->set_hmirror(s, camHFlip);
}
void handleSetCameraVerticalFlip() {
  postBoolHandler("camVFlip", settingsSetCameraVerticalFlip);
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, camVFlip);
}
void handleSetCameraBrightness() {
  postIntHandler("camBrightness", settingsSetCameraBrightness);
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, camBrightness);
}
void handleSetCameraContrast() {
  postIntHandler("camContrast", settingsSetCameraContrast);
  sensor_t *s = esp_camera_sensor_get();
  s->set_contrast(s, camContrast);
}
void handleSetCameraSaturation() {
  postIntHandler("camSaturation", settingsSetCameraSaturation);
  sensor_t *s = esp_camera_sensor_get();
  s->set_saturation(s, camSaturation);
}
void handleSetCameraSharpness() {
  postIntHandler("camSharpness", settingsSetCameraSharpness);
  sensor_t *s = esp_camera_sensor_get();
  s->set_sharpness(s, camSharpness);
}
void handleUpdateReturn() {
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
}
void handleUpdate() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(
            UPDATE_SIZE_UNKNOWN)) { // start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) !=
        upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("Update Success: %u\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}
void handleScanWifi() {
  uint8_t count = wifiScan();
  String json = "[";
  for (uint8_t i=0; i<count; i++) {
    json += "{";
    json += "\"ssid\":\"" + wifiGetScannedSSID(i) + "\",";
    json += "\"rssi\":" + String(wifiGetScannedRSSI(i)) + ",";
    json += "\"secure\":" + String(wifiGetScannedSecure(i)) + ",";
    json += "\"channel\":" + String(wifiGetScannedChannel(i)) + ",";
    json += "\"bssid\":\"" + wifiGetScannedBSSID(i) + "\"";
    json += "},";
  }
  if (count > 0) {
    json.remove(json.length() - 1); // Remove the last comma
  }
  json += "]";
  setCrossOriginHeaders();
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send(200, "application/json", json);
}
void handleSetSta() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  setCrossOriginHeaders();
  if (ssid.length() <= 0 || ssid.length() > 32) {
    server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
    server.send(400, "text/plain", "SSID length should be between 1 and 32.");
    return;
  }
  if (password.length() <= 7 || password.length() > 64) {
    server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
    server.send(400, "text/plain", "Password length should be between 8 and 64.");
    return;
  }
  bool r = wifiConnectSta(ssid, password);
  if (r) {
    settingsSetStaSsid(ssid);
    settingsSetStaPassword(password);
    server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
    server.send(200, "text/plain", wifiGetStaIp());
  } else {
    server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
    server.send(400, "text/plain", "Wi-Fi Connection failed.");
  }
}
void handleRestart() {
  server.sendHeader(HEADER_CONNECTION, HEADER_CONNECTION_CLOSE);
  server.send(200, "text/plain", "OK");
  ESP.restart();
}

void settingsBegin(String _version) {
  version = _version;
  settingsReadConfig();

  server.on("/", HTTP_GET, handleGetIndex);
  server.on("/static/js/main.6ebed670.js", HTTP_GET, handleGetJs);
  server.on("/static/css/main.75b37e2b.css", HTTP_GET, handleGetCss);
  server.on("/favicon.ico", HTTP_GET, handleGetFavicon);
  server.on("/settings", HTTP_GET, handleGetSettings);
  server.on("/set-name", HTTP_POST, handleSetName);
  server.on("/set-type", HTTP_POST, handleSetType);
  server.on("/set-apSsid", HTTP_POST, handleSetApSsid);
  server.on("/set-apPassword", HTTP_POST, handleSetApPassword);
  server.on("/set-apChannel", HTTP_POST, handleSetApChannel);
  server.on("/set-cameraHorizontalMirror", HTTP_POST, handleSetCameraHorizontalMirror);
  server.on("/set-cameraVerticalFlip", HTTP_POST, handleSetCameraVerticalFlip);
  server.on("/set-cameraBrightness", HTTP_POST, handleSetCameraBrightness);
  server.on("/set-cameraContrast", HTTP_POST, handleSetCameraContrast);
  server.on("/set-cameraSaturation", HTTP_POST, handleSetCameraSaturation);
  server.on("/set-cameraSharpness", HTTP_POST, handleSetCameraSharpness);
  server.on("/update", HTTP_POST, handleUpdateReturn, handleUpdate);
  server.on("/scan-wifi", HTTP_GET, handleScanWifi);
  server.on("/set-sta", HTTP_POST, handleSetSta);
  server.on("/restart", HTTP_POST, handleRestart);
  server.begin();
}

void settingsReadConfig() {
  preferences.begin(SETTING_PREFERENCES, false);
  name = preferences.getString("name", DEFAULT_NAME);
  type = preferences.getString("type", DEFAULT_TYPE);
  apSsid = preferences.getString("apSsid", DEFAULT_AP_SSID);
  apPassword = preferences.getString("apPassword", DEFAULT_AP_PASSWORD);
  apChannel = preferences.getInt("apChannel", DEFAULT_AP_CHANNEL);
  staSsid = preferences.getString("staSsid", DEFAULT_STA_SSID);
  staPassword = preferences.getString("staPassword", DEFAULT_STA_PASSWORD);
  camHFlip = preferences.getBool("camHFlip", DEFAULT_CAMERA_HORIZONTAL_MIRROR);
  camVFlip = preferences.getBool("camVFlip", DEFAULT_CAMERA_VERTICAL_FLIP);
  camBrightness = preferences.getInt("camBrightness", DEFAULT_CAMERA_BRIGHTNESS);
  camContrast = preferences.getInt("camContrast", DEFAULT_CAMERA_CONTRAST);
  camSaturation = preferences.getInt("camSaturation", DEFAULT_CAMERA_SATURATION);
  camSharpness = preferences.getInt("camSharpness", DEFAULT_CAMERA_SHARPNESS);
  preferences.end();
}

void settingsReset() {
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.clear();
  preferences.end();
  settingsReadConfig();
}

void settingsLoop() { server.handleClient(); }

String settingsGetName() { return name; }
String settingsGetType() { return type; }
String settingsGetApSsid() { return apSsid; }
String settingsGetApPassword() { return apPassword; }
int settingsGetApChannel() { return apChannel; }
String settingsGetStaSsid() { return staSsid; }
String settingsGetStaPassword() { return staPassword; }
bool settingsGetCameraHorizontalMirror() { return camHFlip; }
bool settingsGetCameraVerticalFlip() { return camVFlip; }
int settingsGetCameraBrightness() { return camBrightness; }
int settingsGetCameraContrast() { return camContrast; }
int settingsGetCameraSaturation() { return camSaturation; }
int settingsGetCameraSharpness() { return camSharpness; }

bool settingsSetName(String value) {
  name = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putString("name", name);
  preferences.end();
  return result;
}

bool settingsSetType(String value) {
  type = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putString("type", type);
  preferences.end();
  return result;
}

bool settingsSetApSsid(String value) {
  apSsid = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putString("apSsid", apSsid);
  preferences.end();
  return result;
}

bool settingsSetApPassword(String value) {
  apPassword = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putString("apPassword", apPassword);
  preferences.end();
  return result;
}

bool settingsSetApChannel(int value) {
  apChannel = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putInt("apChannel", apChannel);
  preferences.end();
  return result;
}

bool settingsSetStaSsid(String value) {
  staSsid = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putString("staSsid", staSsid);
  preferences.end();
  return result;
}

bool settingsSetStaPassword(String value) {
  staPassword = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putString("staPassword", staPassword);
  preferences.end();
  return result;
}

bool settingsSetCameraHorizontalMirror(bool value) {
  camHFlip = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putBool("camHFlip", camHFlip);
  preferences.end();
  return result;
}

bool settingsSetCameraVerticalFlip(bool value) {
  camVFlip = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putBool("camVFlip", camVFlip);
  preferences.end();
  return result;
}

bool settingsSetCameraBrightness(int value) {
  camBrightness = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putInt("camBrightness", camBrightness);
  preferences.end();
  return result;
}

bool settingsSetCameraContrast(int value) {
  camContrast = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putInt("camContrast", camContrast);
  preferences.end();
  return result;
}

bool settingsSetCameraSaturation(int value) {
  camSaturation = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putInt("camSaturation", camSaturation);
  preferences.end();
  return result;
}

bool settingsSetCameraSharpness(int value) {
  camSharpness = value;
  preferences.begin(SETTING_PREFERENCES, false);
  bool result = preferences.putInt("camSharpness", camSharpness);
  preferences.end();
  return result;
}

bool settingsNameChanged() { return name.compareTo(DEFAULT_NAME) != 0; }

bool settingsTypeChanged() { return type.compareTo(DEFAULT_TYPE) != 0; }

bool settingsApSsidChanged() { return apSsid.compareTo(DEFAULT_AP_SSID) != 0; }

bool settingsApPasswordChanged() {
  return apPassword.compareTo(DEFAULT_AP_PASSWORD) != 0;
}

bool settingsStaSsidChanged() {
  return staSsid.compareTo(DEFAULT_STA_SSID) != 0;
}

bool settingsStaPasswordChanged() {
  return staPassword.compareTo(DEFAULT_STA_PASSWORD) != 0;
}
