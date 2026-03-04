#include "wifi_helper.h"

#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include "log.h"

// #define DEBUG
WiFiMulti wifiMulti;

String apIp = "";
String staIp = "";
String macPrefix = "";
String macAddress = "";
bool staConnected = false;

void wifiBegin() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin();
  macAddress = WiFi.macAddress();
  macPrefix = macAddress;
  macPrefix.replace(":", "");
  macPrefix = macPrefix.substring(6, 12);
}

bool wifiConnectSta(String ssid, String password, bool wait) {

#ifdef DEBUG
  Serial.println(F("Connecting to WiFi ..."));
  Serial.print(F("ssid:"));
  Serial.println(ssid);
  Serial.print(F("psk:"));
  Serial.println(password);
#endif

  // Connect to wifi
  wifiMulti.addAP(ssid.c_str(), password.c_str());
  wifiMulti.run();

  // Skip waiting if not required
  if (!wait) {
    return true;
  }

  // Wait for wifi to connect
  //   int count = 0;
  // #ifdef DEBUG
  //   Serial.print("[DEBUG] Connecting.");
  // #endif
  //   while (wifiMulti.run() != WL_CONNECTED) {
  // #ifdef DEBUG
  //     Serial.print(".");
  // #endif
  //     delay(500);
  //     count++;
  //     if (count > 20) {
  // #ifdef DEBUG
  //       Serial.println("");
  //       Serial.println(WiFi.status());
  // #endif
  //       return false;
  //     }
  //   }
  // #ifdef DEBUG
  //   Serial.println("");
  // #endif
  //   staConnected = true;
  //   staIp = WiFi.localIP().toString();
  return true;
}

bool wifiConnectAp(String ssid, String password, int channel) {
  String temp = ssid + '-' + macPrefix;
  WiFi.softAP(temp.c_str(), password.c_str(), channel);
  apIp = WiFi.softAPIP().toString();
  return true;
}

void wifiDisconnect() {
  WiFi.softAPdisconnect(true);
  // WiFiMulti V3 没有 disconnect/stop 方法，直接 disconnect
  staConnected = false;
  staIp = "";
  WiFi.disconnect();
}

int wifiSetHostname(String hostname) { return MDNS.begin(hostname.c_str()); }

int wifiScan() {
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

void wifiCheckSta() {
  if (wifiMulti.run() == WL_CONNECTED) {
    if (staConnected == false) {
      staConnected = true;
      staIp = WiFi.localIP().toString();
      Serial.print("[CONNECTED] wifi sta connected, ip: ");
      Serial.println(staIp);
    }
  } else {
    if (staConnected == true) {
      staConnected = false;
      staIp = "";
      WiFi.disconnect();
      Serial.println("[DISCONNECTED] wifi disconnected");
    }
  }
}

String wifiGetStaIp() { return staIp; }

String wifiGetApIp() { return apIp; }

String wifiGetMacPrefix() { return macPrefix; }

String wifiGetMacAddress() { return macAddress; }

bool wifiIsStaConnected() { return staConnected; }
