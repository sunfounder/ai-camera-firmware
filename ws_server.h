#ifndef __WS_SERVER_H__
#define __WS_SERVER_H__

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

#endif // __WS_SERVER_H__