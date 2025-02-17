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
bool cameraHorizontalMirror;
bool cameraVerticalFlip;
int cameraBrightness;
int cameraContrast;
int cameraSaturation;
int cameraSharpness;

void setCrossOriginHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "OPTIONS, GET, POST, PUT, DELETE");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
}

void returnOk() {
  setCrossOriginHeaders();
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "OK");
}

void returnNameNotFound(String name) {
  String error = String("Bad Request") + name + " not found";
  setCrossOriginHeaders();
  server.sendHeader("Connection", "close");
  server.send(400, "text/plain", error.c_str());
}

void postStringHandler(const char *name, void (*callback)(String)) {
  if (server.hasArg(name)) {
    callback(server.arg(name));
    returnOk();
  } else {
    returnNameNotFound(name);
  }
}
void postIntHandler(const char *name, void (*callback)(int)) {
  if (server.hasArg(name)) {
    int value = server.arg(name).toInt();
    callback(value);
    returnOk();
  } else {
    returnNameNotFound(name);
  }
}
void postBoolHandler(const char *name, void (*callback)(bool)) {
  if (server.hasArg(name)) {
    String value = server.arg(name);
    bool bValue = (value == "true" || value == "1");
    callback(bValue);
    returnOk();
  } else {
    returnNameNotFound(name);
  }
}

void handleGetIndex() {
  setCrossOriginHeaders();
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", index_html_gz, index_html_gz_len);
}

void handleGetJs() {
  setCrossOriginHeaders();
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/javascript", main_ee1290ab_js_gz, main_ee1290ab_js_gz_len);
}

void handleGetCss() {
  setCrossOriginHeaders();
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/css", main_bf514a6e_css_gz, main_bf514a6e_css_gz_len);
}

void handleGetFavicon() {  
  setCrossOriginHeaders();
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Connection", "close");
  server.send_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
}

void handleGetSettings() {
  setCrossOriginHeaders();
  server.sendHeader("Connection", "close");
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
      "\"cameraHorizontalMirror\":" + (cameraHorizontalMirror ? "true" : "false") + "," +
      "\"cameraVerticalFlip\":" + (cameraVerticalFlip ? "true" : "false") + "," +
      "\"cameraBrightness\":" + String(cameraBrightness) + "," +
      "\"cameraContrast\":" + String(cameraContrast) + "," +
      "\"cameraSaturation\":" + String(cameraSaturation) + "," +
      "\"cameraSharpness\":" + String(cameraSharpness) + "," +
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
  postBoolHandler("cameraHorizontalMirror", settingsSetCameraHorizontalMirror);
  sensor_t *s = esp_camera_sensor_get();
  s->set_hmirror(s, cameraHorizontalMirror);
}
void handleSetCameraVerticalFlip() {
  postBoolHandler("cameraVerticalFlip", settingsSetCameraVerticalFlip);
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, cameraVerticalFlip);
}
void handleSetCameraBrightness() {
  postIntHandler("cameraBrightness", settingsSetCameraBrightness);
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, cameraBrightness);
}
void handleSetCameraContrast() {
  postIntHandler("cameraContrast", settingsSetCameraContrast);
  sensor_t *s = esp_camera_sensor_get();
  s->set_contrast(s, cameraContrast);
}
void handleSetCameraSaturation() {
  postIntHandler("cameraSaturation", settingsSetCameraSaturation);
  sensor_t *s = esp_camera_sensor_get();
  s->set_saturation(s, cameraSaturation);
}
void handleSetCameraSharpness() {
  postIntHandler("cameraSharpness", settingsSetCameraSharpness);
  sensor_t *s = esp_camera_sensor_get();
  s->set_sharpness(s, cameraSharpness);
}
void handleUpdateReturn() {
  setCrossOriginHeaders();
  server.sendHeader("Connection", "close");
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
    if (Update.end(true)) { // true to set the size to the current
                            // progress
      Serial.printf("Update Success: %u\nRebooting...\n",
                    upload.totalSize);
      delay(1000);
      ESP.restart();
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
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", json);
}
void handleSetSta() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  if (ssid.length() <= 0 || ssid.length() > 32) {
    server.sendHeader("Connection", "close");
    server.send(400, "text/plain", "SSID length should be between 1 and 32.");
    return;
  }
  if (password.length() <= 7 || password.length() > 64) {
    server.sendHeader("Connection", "close");
    server.send(400, "text/plain", "Password length should be between 8 and 64.");
    return;
  }
  bool r = wifiConnectSta(staSsid, staPassword);
  if (r) {
    settingsSetStaSsid(ssid);
    settingsSetStaPassword(password);
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", wifiGetStaIp());
  } else {
    server.sendHeader("Connection", "close");
    server.send(400, "text/plain", "Wi-Fi Connection failed.");
  }
}

void settingsBegin(String _version) {
  version = _version;
  settingsReadConfig();

  server.on("/", HTTP_GET, handleGetIndex);
  server.on("/static/js/main.ee1290ab.js", HTTP_GET, handleGetJs);
  server.on("/static/css/main.bf514a6e.css", HTTP_GET, handleGetCss);
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
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, handleUpdateReturn, handleUpdate);
  /*handling wifiHelper */
  server.on("/scan-wifi", HTTP_GET, handleScanWifi);
  server.on("/set-sta", HTTP_POST, handleSetSta);
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
  cameraHorizontalMirror = preferences.getBool("cameraHorizontalMirror", DEFAULT_CAMERA_HORIZONTAL_MIRROR);
  cameraVerticalFlip = preferences.getBool("cameraVerticalFlip", DEFAULT_CAMERA_VERTICAL_FLIP);
  cameraBrightness = preferences.getInt("cameraBrightness", DEFAULT_CAMERA_BRIGHTNESS);
  cameraContrast = preferences.getInt("cameraContrast", DEFAULT_CAMERA_CONTRAST);
  cameraSaturation = preferences.getInt("cameraSaturation", DEFAULT_CAMERA_SATURATION);
  cameraSharpness = preferences.getInt("cameraSharpness", DEFAULT_CAMERA_SHARPNESS);
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
bool settingsGetCameraHorizontalMirror() { return cameraHorizontalMirror; }
bool settingsGetCameraVerticalFlip() { return cameraVerticalFlip; }
int settingsGetCameraBrightness() { return cameraBrightness; }
int settingsGetCameraContrast() { return cameraContrast; }
int settingsGetCameraSaturation() { return cameraSaturation; }
int settingsGetCameraSharpness() { return cameraSharpness; }

void settingsSetName(String value) {
  name = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putString("name", name);
  preferences.end();
}

void settingsSetType(String value) {
  type = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putString("type", type);
  preferences.end();
}

void settingsSetApSsid(String value) {
  apSsid = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putString("apSsid", apSsid);
  preferences.end();
}

void settingsSetApPassword(String value) {
  apPassword = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putString("apPassword", apPassword);
  preferences.end();
}

void settingsSetApChannel(int value) {
  apChannel = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putInt("apChannel", apChannel);
  preferences.end();
}

void settingsSetStaSsid(String value) {
  staSsid = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putString("staSsid", staSsid);
  preferences.end();
}

void settingsSetStaPassword(String value) {
  staPassword = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putString("staPassword", staPassword);
  preferences.end();
}

void settingsSetCameraHorizontalMirror(bool value) {
  cameraHorizontalMirror = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putBool("cameraHorizontalMirror", cameraHorizontalMirror);
  preferences.end();
}

void settingsSetCameraVerticalFlip(bool value) {
  cameraVerticalFlip = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putBool("cameraVerticalFlip", cameraVerticalFlip);
  preferences.end();
}

void settingsSetCameraBrightness(int value) {
  cameraBrightness = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putInt("cameraBrightness", cameraBrightness);
  preferences.end();
}

void settingsSetCameraContrast(int value) {
  cameraContrast = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putInt("cameraContrast", cameraContrast);
  preferences.end();
}

void settingsSetCameraSaturation(int value) {
  cameraSaturation = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putInt("cameraSaturation", cameraSaturation);
  preferences.end();
}

void settingsSetCameraSharpness(int value) {
  cameraSharpness = value;
  preferences.begin(SETTING_PREFERENCES, false);
  preferences.putInt("cameraSharpness", cameraSharpness);
  preferences.end();
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
