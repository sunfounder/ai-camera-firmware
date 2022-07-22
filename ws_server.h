#ifndef __WS_SERVER_H__
#define __WS_SERVER_H__

#include <WebSocketsServer.h>

#define REGIONS (char[26]){'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}
#define REGIONS_LENGTH 26
#define WS_BUFFER_SIZE 1024

class WS_Server {
  public:
    WS_Server();
    void begin(int port);
    void loop();
    void send(String data);
    bool is_connected();

  private:
    int port;
};

#endif // __WS_SERVER_H__