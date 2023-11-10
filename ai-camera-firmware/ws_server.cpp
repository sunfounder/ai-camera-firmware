#include "ws_server.h"
#include <ArduinoJson.h>
#include "led_status.hpp"
#include "Ticker.h"

void onWebSocketEvent(uint8_t cn, WStype_t type, uint8_t* payload, size_t length);

WebSocketsServer ws = WebSocketsServer(8765);
uint8_t client_num = 0;
bool ws_connected = false;

String ws_name = "";
String ws_type = "";
String ws_check= "SC";
String videoUrl = "";

Ticker pingPongTimer; // timer for checing ping_pong
bool isPingPingOK = true;
uint32_t lastPingPong = 0;
uint32_t last_pong_time = 0;
uint16_t PONG_INTERVAL = 200;

uint32_t last_send_time = 0;
uint16_t SEND_INTERVAL = 20;

String intToString(uint8_t * value, size_t length) {
  String buf;
  for (int i=0; i<length; i++){
    buf += (char)value[i];
  }
  return buf;
}

void checkPingPong(){
  if (ws_connected == false) {
    return;
  }
  if (millis() - lastPingPong > TIMEOUT) {
    lastPingPong = millis();
    isPingPingOK = false;
    #ifdef DEBUG
    Serial.println("[DEBUG] [WS] PingPong timeout");
    #endif
    Serial.print("[DISCONNECTED] timeout");
  }
}

WS_Server::WS_Server() {}

void WS_Server::close() {
  ws_connected = false;
  ws.close();
  delay(10);
}

void WS_Server::begin(int port, String _name, String _type, String _check) {
  ws_name = _name;
  ws_type = _type;
  ws_check= _check;
  ws = WebSocketsServer(port);
  ws.begin();
  ws.onEvent(onWebSocketEvent);
  pingPongTimer.attach_ms(20, checkPingPong);
}

void WS_Server::loop() {
  ws.loop();
}

void onWebSocketEvent(uint8_t cn, WStype_t type, uint8_t * payload, size_t length) {
  String out;
  client_num = cn;

  // send pong
  // if (ws_connected == true) {
  uint32_t _time = millis();
  if (_time - last_pong_time > PONG_INTERVAL) {
    String msg = "pong "+String(_time);
    ws.sendTXT(client_num, msg);
    last_pong_time = millis();
    #ifdef DEBUG
    Serial.println("[DEBUG] [WS] send PONG");
    #endif
    // Serial.println(msg);
  }
  // }

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
      ws_connected = true;
      break;
    }
    // receive text
    case WStype_TEXT:{
      ws_connected = true;

      out = intToString(payload, length);

      // reset ping_pong time
      lastPingPong = millis();
      if (strcmp(out.c_str(), "ping") == 0) {
        Serial.println("[APPSTOP]");
        return;
      }

      // ------------- send simplified text -------------
      DynamicJsonDocument recvBuffer(WS_BUFFER_SIZE);
      deserializeJson(recvBuffer, out);
      String result = "WS+";

      // REGIONS
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

        if (value == "true") value = "1";
        else if (value == "false") value = "0";
        if (value != "null") result += value;
        if (i != REGIONS_LENGTH - 1) result += ';';
      }

      // send
      if (millis() - last_send_time > SEND_INTERVAL ) {
        Serial.println(result);
        last_send_time = millis();
      }
      break;
    }
    // For everything else: do nothing
    case WStype_BIN: {
      Serial.print("WSB+");
      Serial.write(payload, length); Serial.println();
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_BIN");
      #endif
      break;
    }
    case WStype_ERROR: {
      LED_STATUS_ERROR();
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

void WS_Server::send(String data) {
  ws.sendTXT(client_num, data);
}

// https://github.com/Links2004/arduinoWebSockets/blob/master/src/WebSocketsServer.cpp#L230
void WS_Server::sendBIN(uint8_t* payload, size_t length) {
  // bool WebSocketsServerCore::sendBIN(uint8_t num, const uint8_t * payload, size_t length)
  ws.sendBIN(client_num, payload, length);
}


bool WS_Server::isConnected() {
  return ws_connected;
}

