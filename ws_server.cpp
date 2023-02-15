#include "ws_server.h"
#include <ArduinoJson.h>
#include "led_status.hpp"

WebSocketsServer ws = WebSocketsServer(8765);
uint8_t client_num = 0;
bool ws_connected = false;
// #define DEBUG
String ws_name = "";
String ws_type = "";
String ws_check= "SC";
extern String videoUrl;

bool isPingPingOK = true;
uint32_t lastPingPong = 0;
hw_timer_t * pingPongTimer = NULL;

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
      LED_STATUS_DISCONNECTED();
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] Disconnected!");
      #endif
      IPAddress remoteIp = ws.remoteIP(client_num);
      Serial.print("[DISCONNECTED] ");Serial.println(remoteIp.toString());
      ws_connected = false;
      break;
    }
    // New client has connected
    case WStype_CONNECTED:{
      LED_STATUS_CONNECTED();
      IPAddress remoteIp = ws.remoteIP(client_num);
      #ifdef DEBUG
      Serial.print("[DEBUG] [WS] Connection from ");
      Serial.println(remoteIp.toString());
      #endif
      Serial.print("[CONNECTED] ");Serial.println(remoteIp.toString());
      // Send check_info  to client
      String check_info = "{\"Name\":\"" + ws_name
                        + "\",\"Type\":\"" + ws_type
                        + "\",\"Check\":\"" + ws_check
                        + "\",\"video\":\"" + videoUrl
                        + "\"}";
      ws.sendTXT(client_num, check_info);
      delay(100);
      ws.sendTXT(client_num, check_info);

      // out = serialReadBlock();
      // ws.sendTXT(client_num, out);
      ws_connected = true;
      break;
    }
    case WStype_TEXT:{
      // #ifdef DEBUG
      // Serial.printf("[DEBUG] [%u] Received text: ", client_num);
      // #endif
      out = intToString(payload, length);
      if (strcmp(out.c_str(), "ping") == 0) {
        lastPingPong = millis();
        ws.sendTXT(client_num, "pong");
          isPingPingOK = true;
        #ifdef DEBUG
        Serial.println("[DEBUG] [WS] PIN PONG");
        #endif
      } else {
        // #ifdef DEBUG
        // Serial.print("[DEBUG] [WS] Received text: ");Serial.println(out);
        // #endif
        DynamicJsonDocument recvBuffer(WS_BUFFER_SIZE);
        deserializeJson(recvBuffer, out);
        String result = "WS+";
        for (int i=0; i<REGIONS_LENGTH; i++){
          String region = String(REGIONS[i]);
          String value;
          if (recvBuffer[region].is<JsonArray>()) {
            for (int j=0; j<recvBuffer[region].size(); j++) {
              value += recvBuffer[region][j].as<String>();
              if (j != recvBuffer[region].size()-1) value += ',';
            }
          } else {
            value = recvBuffer[region].as<String>();
          }
          // #ifdef DEBUG
          // Serial.print(region);Serial.print(": ");Serial.println(value);
          // #endif
          if (value == "true") value = "1";
          else if (value == "false") value = "0";
          if (value != "null") result += value;
          if (i != REGIONS_LENGTH - 1) result += ';';
        }
        // #ifdef DEBUG
        // Serial.print("[DEBUG] [WS] Send over serial: ");
        // #endif
        Serial.println(result);
      }
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
      LED_STATUS_ERROOR();
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

void ARDUINO_ISR_ATTR checkPingPong(){
  if (ws_connected == false) {
    return;
  }
  if (millis() - lastPingPong > TIMEOUT) {
    isPingPingOK = false;
    #ifdef DEBUG
    Serial.println("[DEBUG] [WS] PingPong timeout");
    #endif
    Serial.print("[DISCONNECTED] timeout");
  }
}

void WS_Server::begin(int port, String _name, String _type, String _check) {
  ws_name = _name;
  ws_type = _type;
  ws_check= _check;
  ws = WebSocketsServer(port);
  ws.begin();
  ws.onEvent(onWebSocketEvent);
  pingPongTimer = timerBegin(0, 65535, true); // prescaler 8000, max 65535, 80Mhz / 65535 = 1220.3125Hz = 0.0008192s
  timerAttachInterrupt(pingPongTimer, &checkPingPong, true);
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

bool WS_Server::is_connected() {
  return ws_connected;
}

