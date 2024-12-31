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
    bool connectAp(String ssid, String password, int channel = 1); 
    bool connectSta(String ssid, String password);
    uint8_t scan();
    void scanClean();
    String getScanedSSID(uint8_t index);
    int32_t getScanedRSSI(uint8_t index);
    uint8_t getScanedSecure(uint8_t index);
    int32_t getScanedChannel(uint8_t index);
    String getScanedBSSID(uint8_t index);
  private:
    String macAddress;
    String macPrefix;
};

#endif