
// Mode
#define NONE 0
#define STA 1
#define AP 2

class WiFiHelper {
  public:
    WiFiHelper();
    bool connect(int mode, String ssid, String password);
    String ip = "";
    String ssid = "";
    String password = "";
    bool is_connected = false;

  private:
    bool connect_AP(); 
    bool connect_STA();
};