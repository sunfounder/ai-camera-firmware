#ifndef __WIFI_HELPER_H__
#define __WIFI_HELPER_H__

#include <WiFiMulti.h>
#include <WiFi.h>

// Mode
#define NONE 0
#define STA 1
#define AP 2

class WiFiHelper {
  public:
    WiFiHelper();
    String ip = "";
    bool staConnected = false;
    bool isConnected = false;

    void checkSta();
    bool connectAp(String ssid, String password); 
    bool connectSta(String ssid, String password);
};

#endif