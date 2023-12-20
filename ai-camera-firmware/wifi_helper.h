#ifndef __WIFI_HELPER_H__
#define __WIFI_HELPER_H__

#include <WiFi.h>

class WiFiHelper {
  public:
    WiFiHelper();
    String apIp = "";
    String staIp = "";
    bool staConnected = false;
    bool isConnected = false;

    void begin();
    void checkSta();
    bool connectAp(String ssid, String password); 
    bool connectSta(String ssid, String password);
};

#endif