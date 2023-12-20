#include "wifi_helper.h"

WiFiHelper::WiFiHelper() {}

void WiFiHelper::begin(){
  WiFi.mode(WIFI_AP_STA);
}

bool WiFiHelper::connectSta(String ssid, String password){

  Serial.println(F("Connecting to WiFi ..."));
  Serial.print(F("ssid:"));Serial.println(ssid);
  Serial.print(F("psk:"));Serial.println(password);

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

bool WiFiHelper::connectAp(String ssid, String password){
  WiFi.softAP(ssid.c_str(), password.c_str());
  apIp = WiFi.softAPIP().toString();
  isConnected = true;
  return true;
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
