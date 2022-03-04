#include <WebSocketsServer.h>

class WS_Server {
  public:
    WS_Server();
    void begin(int port);
    void loop();
    void send(String data);

  private:
    int port;
};

