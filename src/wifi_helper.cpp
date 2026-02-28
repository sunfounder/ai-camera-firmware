#include "wifi_helper.h"

#define DEBUG

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
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());

  // Skip waiting if not required
  if (!wait) {
    return true;
  }

  // Wait for wifi to connect
  int count = 0;
#ifdef DEBUG
  Serial.print("[DEBUG] Connecting.");
#endif
  while (WiFi.status() != WL_CONNECTED) {
#ifdef DEBUG
    Serial.print(".");
#endif
    delay(500);
    count++;
    if (count > 20) {
#ifdef DEBUG
      Serial.println("");
      Serial.println(WiFi.status());
#endif
      return false;
    }
  }
#ifdef DEBUG
  Serial.println("");
#endif
  staConnected = true;
  staIp = WiFi.localIP().toString();
  return true;
}

bool wifiConnectAp(String ssid, String password, int channel) {
  String temp = ssid + '-' + macPrefix;
  Serial.println(temp);
  WiFi.softAP(temp.c_str(), password.c_str(), channel);
  apIp = WiFi.softAPIP().toString();
  return true;
}

int wifiSetHostname(String hostname) { return MDNS.begin(hostname); }

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
  if (WiFi.status() == WL_CONNECTED) {
    if (staConnected == false) {
      staConnected = true;
      staIp = WiFi.localIP().toString();
      Serial.println("[CONNECTED] wifi sta connected");
    }
  } else {
    if (staConnected == true) {
      staConnected = false;
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
