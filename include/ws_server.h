#ifndef __WS_SERVER_H__
#define __WS_SERVER_H__

#include <WebSocketsServer.h>
#include "wifi_helper.h"

// #define DEBUG

#define REGIONS (char[26]){'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}
#define REGIONS_LENGTH 26
#define WS_BUFFER_SIZE 1024

#define TIMEOUT 2000

class WS_Server {
  public:
    WS_Server();
    void begin(int port);
    void begin(int port, String name, String type, String check);
    void close();
    void loop();
    void send(String data);
    void sendBIN(uint8_t* payload, size_t length);
    bool isConnected();
    void setStaIp(String ip);
  private:
    int port;
};

#endif // __WS_SERVER_H__