#include "wifi_helper.h"

#include <ESPmDNS.h>
#include <WiFi.h>

#include "log.h"

#define DEBUG

String macPrefix = "";
String macAddress = "";

void wifiBegin() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin();
  WiFi.setAutoReconnect(false);
  String macAddress = WiFi.macAddress();
  macPrefix = macAddress;
  macPrefix.replace(":", "");
  macPrefix = macPrefix.substring(6, 12);
}

bool wifiConnectSta(String ssid, String password, uint8_t waitSecond) {

#ifdef DEBUG
  Serial.println(F("Connecting to WiFi ..."));
  Serial.print(F("ssid:"));
  Serial.println(ssid);
  Serial.print(F("psk:"));
  Serial.println(password);
#endif

  // Connect to wifi
  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait for connection
  for (int i = 0; i < waitSecond; i++) {
    if (wifiIsStaConnected()) {
      break;
    }
    delay(1000);
  }

  return wifiIsStaConnected();
}

bool wifiConnectAp(String ssid, String password, int channel) {
  String temp = ssid + '-' + macPrefix;
  WiFi.softAP(temp.c_str(), password.c_str(), channel);
  return true;
}

void wifiDisconnect() {
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true, true); // 关闭 WiFi 并清除配置
}

int wifiSetHostname(String hostname) { return MDNS.begin(hostname.c_str()); }

int wifiScan() {
#ifdef DEBUG
  Serial.println(F("Scanning WiFi ..."));
#endif
  int count = 0;
  WiFi.disconnect();
  WiFi.scanDelete();
  count = WiFi.scanNetworks();
  return count;
}

void wifiScanClean() { WiFi.scanDelete(); }

String wifiGetScannedSSID(int index) { return WiFi.SSID(index); }

int32_t wifiGetScannedRSSI(int index) { return WiFi.RSSI(index); }

uint8_t wifiGetScannedSecure(int index) { return WiFi.encryptionType(index); }

int32_t wifiGetScannedChannel(int index) { return WiFi.channel(index); }

String wifiGetScannedBSSID(int index) { return WiFi.BSSIDstr(index); }

String wifiGetStaIp() { return WiFi.localIP().toString(); }

String wifiGetApIp() { return WiFi.softAPIP().toString(); }

String wifiGetMacPrefix() { return macPrefix; }

String wifiGetMacAddress() { return WiFi.macAddress(); }

bool wifiIsStaConnected() { return WiFi.status() == WL_CONNECTED; }
