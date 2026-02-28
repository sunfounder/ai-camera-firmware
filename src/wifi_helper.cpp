#include "wifi_helper.h"

#define DEBUG

String apIp = "";
String staIp = "";
String macPrefix = "";
String macAddress = "";
bool staConnected = false;
bool isConnected = false;

void wifiBegin(){
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin();
  macAddress = WiFi.macAddress();
  macPrefix = macAddress;
  macPrefix.replace(":", "");
  macPrefix = macPrefix.substring(6, 12);
}

bool wifiConnectSta(String ssid, String password){

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

bool wifiConnectAp(String ssid, String password, int channel){
  String temp = ssid + '-' + macPrefix;
  Serial.println(temp);
  WiFi.softAP(temp.c_str(), password.c_str(), channel);
  apIp = WiFi.softAPIP().toString();
  isConnected = true;
  return true;
}

int wifiSetHostname(String hostname){
  return MDNS.begin(hostname);
}

uint8_t wifiScan() { return WiFi.scanNetworks(); }

void wifiScanClean() { WiFi.scanDelete(); }

String wifiGetScannedSSID(uint8_t index) { return WiFi.SSID(index); }

int32_t wifiGetScannedRSSI(uint8_t index) { return WiFi.RSSI(index); }

uint8_t wifiGetScannedSecure(uint8_t index) {
  return WiFi.encryptionType(index);
}

int32_t wifiGetScannedChannel(uint8_t index) {
  return WiFi.channel(index);
}

String wifiGetScannedBSSID(uint8_t index) {
  return WiFi.BSSIDstr(index);
}

void wifiCheckSta(){
  if (WiFi.status() != WL_CONNECTED) {
    if (isConnected == true) {
      isConnected = false;
      staConnected = false;
      WiFi.disconnect();
      Serial.println("[DISCONNECTED] wifi disconnected");
    }
  }
}

String wifiGetStaIp(){
  return staIp;
}

String wifiGetApIp(){
  return apIp;
}

String wifiGetMacPrefix(){
  return macPrefix;
}

String wifiGetMacAddress(){
  return macAddress;
}

bool wifiIsStaConnected(){
  return staConnected;
}

bool wifiIsConnected(){
  return isConnected;
}