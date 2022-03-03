#include <WebSocketsServer.h>

class WS_Server {
  public:
    WS_Server();
    void loop();
    bool is_connected();

  private:
    String ssid, password;
    int mode;
    String rxBuf = "";
    int port;

    String serialRead();
    void handleSet(String cmd);
    void handleGet(String cmd);
};

