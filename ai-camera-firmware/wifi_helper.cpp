#include "wifi_helper.h"

WiFiHelper::WiFiHelper() {}

void WiFiHelper::begin(){
  WiFi.mode(WIFI_AP_STA);
  macAddress = WiFi.macAddress();
  macPrefix = macAddress;
  macPrefix.replace(":", "");
  macPrefix = macPrefix.substring(6, 12);
}

bool WiFiHelper::connectSta(String ssid, String password){

  #ifdef DEBUG
  Serial.println(F("Connecting to WiFi ..."));
  Serial.print(F("ssid:"));Serial.println(ssid);
  Serial.print(F("psk:"));Serial.println(password);
  #endif

  // Connect to wifi
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait some time to connect to wifi
  int count = 0;
  #ifdef DEBUG
  Serial.print("[DEBUG] Connecting.");
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG
    Serial.print("."); 
    #endif
    delay(500);
    count ++;
    if (count > 20){
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
  isConnected = true;
  staConnected = true;
  staIp = WiFi.localIP().toString();
  return true;
}

bool WiFiHelper::connectAp(String ssid, String password, int channel){
  String temp = ssid + '-' + macPrefix;
  WiFi.softAP(temp.c_str(), password.c_str(), channel);
  apIp = WiFi.softAPIP().toString();
  isConnected = true;
  return true;
}

uint8_t WiFiHelper::scan() { return WiFi.scanNetworks(); }

void WiFiHelper::scanClean() { WiFi.scanDelete(); }

String WiFiHelper::getScanedSSID(uint8_t index) { return WiFi.SSID(index); }

int32_t WiFiHelper::getScanedRSSI(uint8_t index) { return WiFi.RSSI(index); }

uint8_t WiFiHelper::getScanedSecure(uint8_t index) {
  return WiFi.encryptionType(index);
}

int32_t WiFiHelper::getScanedChannel(uint8_t index) {
  return WiFi.channel(index);
}

String WiFiHelper::getScanedBSSID(uint8_t index) {
  return WiFi.BSSIDstr(index);
}

void WiFiHelper::checkSta(){
  if (WiFi.status() != WL_CONNECTED) {
    if (isConnected == true) {
      isConnected = false;
      staConnected = false;
      WiFi.disconnect();
      Serial.println("[DISCONNECTED] wifi disconnected");
    }
  }
}
