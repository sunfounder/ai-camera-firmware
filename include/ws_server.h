#ifndef __WS_SERVER_H__
#define __WS_SERVER_H__

#include "wifi_helper.h"
#include <WebSocketsServer.h>

#define REGIONS                                                                \
  (char[26]){'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',  \
             'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'}
#define REGIONS_LENGTH 26
#define WS_BUFFER_SIZE 1024

#define TIMEOUT 2000
#define MAX_CLIENTS 5  // 最大支持 5 个客户端

class WS_Server {
public:
  WS_Server();
  void begin(int port);
  void begin(int port, String name, String type, String check);
  void close();
  void loop();
  void send(String data);
  void sendBIN(uint8_t *payload, size_t length);
  bool isConnected();
  void setStaIp(String ip);
  int getClientCount();

private:
  int port;
};

#endif // __WS_SERVER_H__