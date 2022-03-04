#include "ws_server.h"
#include <ArduinoJson.h>
#include "wifi_helper.h"

#define SERIAL_TIMEOUT 100 // 100ms

WiFiHelper wifi = WiFiHelper();
WebSocketsServer ws = WebSocketsServer(8765);
uint8_t client_num = 0;

WS_Server::WS_Server() {}

String intToString(uint8_t * value, size_t length) {
  String buf;
  for (int i=0; i<length; i++){
    buf += (char)value[i];
  }
  return buf;
}

static void onWebSocketEvent(uint8_t cn, WStype_t type, uint8_t * payload, size_t length) {
  String out;
  client_num = cn;
  #ifdef DEBUG
  Serial.println("[DEBUG] onWebSocketEvent");
  #endif
  switch(type) {
    // Client has disconnected
    case WStype_DISCONNECTED:{
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] Disconnected!");
      #endif
      IPAddress remoteIp = ws.remoteIP(client_num);
      Serial.print("[DISCONNECTED] ");Serial.println(remoteIp.toString());
      break;
    }
    // New client has connected
    case WStype_CONNECTED:{
      IPAddress remoteIp = ws.remoteIP(client_num);
      #ifdef DEBUG
      Serial.print("[DEBUG] [WS] Connection from ");
      Serial.println(remoteIp.toString());
      #endif
      Serial.print("[CONNECTED] ");Serial.println(remoteIp.toString());
      // out = serialReadBlock();
      // ws.sendTXT(client_num, out);
      break;
    }
    case WStype_TEXT:{
      // #ifdef DEBUG
      // Serial.printf("[DEBUG] [%u] Received text: ", client_num);
      // #endif
      out = intToString(payload, length);
      #ifdef DEBUG
      Serial.print("[DEBUG] [WS] Received text: ");Serial.println(out);
      #endif
      Serial.print("WS+");
      Serial.println(out);
      // out = serialReadBlock();
      // #ifdef DEBUG
      // Serial.print("[DEBUG] Read from Serial: ");Serial.println(out);
      // #endif
      // ws.sendTXT(client_num, out);
      break;
    }
    // For everything else: do nothing
    case WStype_BIN: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_BIN");
      #endif
      break;
    }
    case WStype_ERROR: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_ERROR");
      #endif
      break;
    }
    case WStype_FRAGMENT_TEXT_START: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_FRAGMENT_TEXT_START");
      #endif
      break;
    }
    case WStype_FRAGMENT_BIN_START: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_FRAGMENT_BIN_START");
      #endif
      break;
    }
    case WStype_FRAGMENT: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_FRAGMENT");
      #endif
      break;
    }
    case WStype_FRAGMENT_FIN: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_FRAGMENT_FIN");
      #endif
      break;
    }
    case WStype_PING: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_PING");
      #endif
      break;
    }
    case WStype_PONG: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_PONG");
      #endif
      break;
    }
    default: {
      #ifdef DEBUG
      Serial.print("[DEBUG] [WS] Event Type: [");Serial.print(type);Serial.println("]");
      #endif
      break;
    }
  }
}

void WS_Server::begin(int port) {
  ws = WebSocketsServer(port);
  ws.begin();
  ws.onEvent(onWebSocketEvent);
  #ifdef DEBUG
  Serial.println("[DEBUG] WebSocket server started");
  #endif
}

void WS_Server::loop() {
  ws.loop();
}

void WS_Server::send(String data) {
  ws.sendTXT(client_num, data);
}
