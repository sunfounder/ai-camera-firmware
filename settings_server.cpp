#include "settings_server.hpp"
#include "settings_server_index_html.hpp"

WebServer server(80);
char version[10] = "";
int _apChannel = 1;

void settingsBegin(const char* _version, int chn) {
  _apChannel = chn;
  strcpy(version, _version);
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Content-Encoding", "gzip");
    server.send_P(200, "text/html", (const char*)FPSTR(setting_server_index_html_gz), setting_server_index_html_gz_len);
  });
  server.on("/settings", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "application/json", "{\"version\":\"" + String(version) + "\",\"apChannel\":" + String(_apChannel) + "}");
  });
  server.on("/version", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", version);
  });
  server.on("/apChannel", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", String(_apChannel));
  });
  server.on("/set-apChannel", HTTP_POST, []() {
    if (server.hasArg("apChannel")) {
      _apChannel = server.arg("apChannel").toInt();
      Preferences pref;
      pref.begin("config");
      pref.putInt("apChannel", _apChannel);
      pref.end();
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", "OK");
    } else {
      server.sendHeader("Connection", "close");
      server.send(400, "text/plain", "Bad Request");
    }
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        delay(1000);
        ESP.restart();
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void settingsLoop() {
  server.handleClient();
}