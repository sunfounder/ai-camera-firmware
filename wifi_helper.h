#ifndef __WIFI_HELPER_H__
#define __WIFI_HELPER_H__

// Mode
#define NONE 0
#define STA 1
#define AP 2

class WiFiHelper {
  public:
    WiFiHelper();
    bool connect(int mode, String ssid, String password, int apChannel = 1);
    String ip = "";
    String ssid = "";
    String password = "";
    int apChannel = 1;
    bool is_connected = false;
    void check_status();

  private:
    bool connect_AP(); 
    bool connect_STA();
};

#endif