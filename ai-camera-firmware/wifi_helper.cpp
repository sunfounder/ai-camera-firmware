
#include <WiFi.h>
#include "wifi_helper.h"

WiFiHelper::WiFiHelper() {
}

bool WiFiHelper::connect_STA(){

  Serial.println(F("Connecting to WiFi ..."));
  Serial.print(F("ssid:"));Serial.println(ssid);
  Serial.print(F("psk:"));Serial.println(password);

  // Connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait some time to connect to wifi
  int count = 0;
  Serial.print("[DEBUG] Connecting.");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); 
    delay(500);
    count ++;
    if (count > 30){
      Serial.println("");
      Serial.println(WiFi.status());
      return false;
    }
  }
  Serial.println("");
  ip = WiFi.localIP().toString();
  return true;
}

bool WiFiHelper::connect_AP(){
  WiFi.softAP(ssid.c_str(), password.c_str());
  ip = WiFi.softAPIP().toString();
  return true;
}

bool WiFiHelper::connect(int mode, String _ssid, String _password){
  bool ret;
  ssid = _ssid;
  password = _password;

  #ifdef DEBUG
  Serial.print("[DEBUG] Mode:");
  #endif
  if (mode == AP){
    #ifdef DEBUG
    Serial.println("AP");
    #endif
    ret = connect_AP();
  } else if (mode == STA) {
    #ifdef DEBUG
    Serial.println("STA");
    #endif
    ret = connect_STA();
  }

  if (!ret){
    return false;
  }

  #ifdef DEBUG
  Serial.print("[DEBUG] IP address: ");Serial.println(ip);
  #endif

  is_connected = true;
  return true;
}

void WiFiHelper::check_status(){
  if (WiFi.status() != WL_CONNECTED) {
    if (is_connected == true) {
      is_connected = false;
      WiFi.disconnect();
      Serial.println("[DISCONNECTED] wifi disconnected");
    }
  }
}
