#include "ws_server.h"
#include <ArduinoJson.h>
#include "wifi_helper.h"

#define SERIAL_TIMEOUT 100 // 100ms

WiFiHelper wifi = WiFiHelper();
WebSocketsServer webSocket = WebSocketsServer(8765);
uint8_t client_num = 0;

WS_Server::WS_Server() {}

String intToString(uint8_t * value, size_t length) {
  // #ifdef DEBUG
  // Serial.println("[DEBUG] intToString");
  // #endif
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
      IPAddress remoteIp = webSocket.remoteIP(client_num);
      Serial.print("[DISCONNECTED] ");Serial.println(remoteIp.toString());
      break;
    }
    // New client has connected
    case WStype_CONNECTED:{
      IPAddress remoteIp = webSocket.remoteIP(client_num);
      #ifdef DEBUG
      Serial.print("[DEBUG] [WS] Connection from ");
      Serial.println(remoteIp.toString());
      #endif
      Serial.print("[CONNECTED] ");Serial.println(remoteIp.toString());
      // out = serialReadBlock();
      // webSocket.sendTXT(client_num, out);
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
      Serial.println(out);
      // out = serialReadBlock();
      // #ifdef DEBUG
      // Serial.print("[DEBUG] Read from Serial: ");Serial.println(out);
      // #endif
      // webSocket.sendTXT(client_num, out);
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

void WS_Server::loop() {
  rxBuf = serialRead();
  if (rxBuf.length() > 0) {
    #ifdef DEBUG
    Serial.print("[DEBUG] RX Receive: ");Serial.println(rxBuf);
    #endif
    delay(10);
    if (rxBuf.substring(0, 4) == "SET+"){
      handleSet(rxBuf.substring(4));
    } else if (rxBuf.substring(0, 3) == "WS+"){
      String out = rxBuf.substring(3);
      #ifdef DEBUG
      Serial.print("[DEBUG] Read from Serial: ");Serial.println(out);
      #endif
      webSocket.sendTXT(client_num, out);
    }
  }
  if (wifi.is_connected){
    webSocket.loop();
  }
  delay(10);
}

String WS_Server::serialRead() {
  String buf = "";
  char inChar;
  int temp;
  unsigned long timeoutStart = millis();
  while (Serial.available() && millis() - timeoutStart < SERIAL_TIMEOUT) {
    temp = Serial.read();
    inChar = (char)temp;
    if (inChar == '\n') {
      break;
    } else if((int)inChar != 255){
      buf += inChar;
    }
  }
  return buf;
}

void WS_Server::handleSet(String cmd){
  if (cmd.substring(0, 4) == "SSID"){
    ssid = cmd.substring(4);
    #ifdef DEBUG
    Serial.print("[DEBUG] Set SSID: ");Serial.println(ssid);
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 3) == "PSK"){
    password = cmd.substring(3);
    #ifdef DEBUG
    Serial.print("[DEBUG] Set password: ");Serial.println(password);
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "PORT"){
    port = cmd.substring(4).toInt();
    #ifdef DEBUG
    Serial.print("[DEBUG] Set port: ");Serial.println(port);
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 4) == "MODE"){
    mode = cmd.substring(4).toInt();
    #ifdef DEBUG
    Serial.print("[DEBUG] Set mode: ");
    if (mode == AP){
      Serial.println("AP");
    } else if (mode == STA) {
      Serial.println("STA");
    }
    #endif
    Serial.println("[OK]");
  } else if (cmd.substring(0, 5) == "RESET"){
    #ifdef DEBUG
    Serial.println("[DEBUG] Reset");
    #endif
    delay(10);
    ESP.restart();
  } else if (cmd.substring(0, 5) == "START"){
    if (ssid.length() == 0) {
      Serial.println("[ERROR] Please set ssid");
    } else if (password.length() == 0) {
      Serial.println("[ERROR] Please set password");
    } else if (mode == NONE) {
      Serial.println("[ERROR] Please set mode");
    } else if (port == 0) {
      Serial.println("[ERROR] Please set port");
    } else{
      bool result = wifi.connect(mode, ssid, password);
      if (!result) {
        Serial.println("[ERROR] TIMEOUT");
      } else {
        webSocket = WebSocketsServer(port);
        webSocket.begin();
        webSocket.onEvent(onWebSocketEvent);
        #ifdef DEBUG
        Serial.println("[DEBUG] Websocket on!");
        #endif
        Serial.print("[OK] ");Serial.println(wifi.ip);
      }
    }
  }
}

void WS_Server::handleGet(String cmd){
  if (cmd.substring(0, 4) == "IP"){
    Serial.println(wifi.ip);
  } else if (cmd.substring(0, 6) == "STATUS") {
    Serial.println(WiFi.status() != WL_CONNECTED);
  }
}

bool WS_Server::is_connected(){
  return wifi.is_connected;
}
