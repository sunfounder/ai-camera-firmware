#include "ws_server.h"
#include <ArduinoJson.h>
#include "led_status.hpp"
#include "Ticker.h"

void onWebSocketEvent(uint8_t cn, WStype_t type, uint8_t* payload, size_t length);

WebSocketsServer ws = WebSocketsServer(8765);
uint8_t client_num = 0;
bool wsConnected = false;

String wsName = "";
String wsType = "";
String wsCheck= "SC";
String videoUrl = "";
String videoTemplate = "";
String staIp = "";

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
  if (wsConnected == false) {
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
  wsConnected = false;
  ws.close();
  delay(10);
}

void WS_Server::begin(int port, String _name, String _type, String _check) {
  wsName = _name;
  wsType = _type;
  wsCheck= _check;
  ws = WebSocketsServer(port);
  ws.begin();
  ws.onEvent(onWebSocketEvent);
  pingPongTimer.attach_ms(20, checkPingPong);
}

void WS_Server::loop() {
  ws.loop();
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
  return wsConnected;
}

void WS_Server::setStaIp(String ip) {
  staIp = ip;
}

void handleConfig(String payload) {
  // Serial.println("SET+ config from websocket");
  DynamicJsonDocument config(WS_BUFFER_SIZE);
  DynamicJsonDocument result(WS_BUFFER_SIZE);
  deserializeJson(config, payload);

  Preferences prefs;
  prefs.begin("config");
  result["state"] = F("ERROR");
  JsonArray errors = result.createNestedArray("errors");
  JsonObject data = result.createNestedObject("data");

  // Get name
  if (config.containsKey("name")) {
    String name = config["name"].as<String>();
    prefs.putString("name", name.c_str());
    data["name"] = name;
    result["state"] = F("OK");
    // Serial.print("set name: ");Serial.println(name);
  }
  if (config.containsKey("type")) {
    String type = config["type"].as<String>();
    prefs.putString("type", type.c_str());
    result["state"] = F("OK");
    // Serial.print("set type: ");Serial.println(type);
  }
  if (config.containsKey("apSsid")) {
    String ap_ssid = config["apSsid"].as<String>();
    prefs.putString("apSsid", ap_ssid.c_str());
    String tmp = prefs.getString("apSsid");
    result["state"] = F("OK");
    // Serial.print("set apSsid: ");Serial.println(ap_ssid);
  }
  if (config.containsKey("apPassword")) {
    String ap_password = config["apPassword"].as<String>();
    prefs.putString("apPassword", ap_password.c_str());
    result["state"] = F("OK");
    // Serial.print("set apPassword: ");Serial.println(ap_password);
  }
  if (config.containsKey("staSsid")) {
    String wifi_ssid = config["staSsid"].as<String>();
    prefs.putString("staSsid", wifi_ssid.c_str());
    result["state"] = F("OK");
    // Serial.print("set staSsid: ");Serial.println(wifi_ssid);
  }
  if (config.containsKey("staPassword")) {
    String wifi_password = config["staPassword"].as<String>();
    prefs.putString("staPassword", wifi_password.c_str());
    result["state"] = F("OK");
    // Serial.print("set staPassword: ");Serial.println(wifi_password);
  }
  if (config.containsKey("command")) {
    String command = config["command"].as<String>();
    if (command == "restart-sta") {
      // Serial.println("restart-sta");
      String staSsid = prefs.getString("staSsid");
      String staPassword = prefs.getString("staPassword");
      WiFiHelper wifi;
      if (staSsid.length() > 0 && staPassword.length() > 0) {
        bool r = wifi.connectSta(staSsid, staPassword);
        if (r) {
          result["state"] = F("OK");
          result["ip"] = wifi.staIp;
          // Serial.print("STA IP: ");Serial.println(result["ip"].as<String>());
          staIp = wifi.staIp;
        } else {
          result["state"] = F("ERROR");
          errors.add(F("STA_CONNECT_ERROR"));
        }
      } else {
        result["state"] = F("ERROR");
        errors.add(F("STA_NOT_CONFIGURED"));

      }
    } else {
      result["state"] = F("ERROR");
      errors.add(F("UNKNOWN_COMMAND"));
    }
  }
  String result_str;
  serializeJson(result, result_str);
  // Serial.print("result_str: ");Serial.println(result_str);
  ws.sendTXT(client_num, result_str);
}

void handleSunFounderController(String payload) {
  // ------------- send simplified text -------------
  DynamicJsonDocument recvBuffer(WS_BUFFER_SIZE);
  deserializeJson(recvBuffer, payload);
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
}

void onWebSocketEvent(uint8_t cn, WStype_t type, uint8_t * payload, size_t length) {
  String out;
  client_num = cn;

  // send pong
  // if (wsConnected == true) {
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
      wsConnected = false;
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
      String check_info = "{\"Name\":\"" + wsName
                        + "\",\"Type\":\"" + wsType
                        + "\",\"Check\":\"" + wsCheck
                        + "\",\"video\":\"" + videoUrl
                        + "\",\"StaIp\":\"" + staIp
                        + "\",\"VideoTemplate\":\"" + videoTemplate
                        + "\"}";
      // ws.sendTXT(client_num, check_info);
      delay(100);
      ws.sendTXT(client_num, check_info);
      wsConnected = true;
      break;
    }
    // receive text
    case WStype_TEXT:{
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_TEXT");
      #endif
      wsConnected = true;
      // Serial.print("WStype_TEXT, length: ");Serial.println(length);

      out = intToString(payload, length);

      // reset ping_pong time
      lastPingPong = millis();
      if (out.compareTo("ping") == 0) {
      // if (strcmp(out.c_str(), "ping") == 0) {
        Serial.println("[APPSTOP]");
        return;
      }
      if (out.startsWith("SET+")) {
        handleConfig(out.substring(4));
        return;
      }
      if (length > 0 ) {
        handleSunFounderController(out);
        return;
      }
      break;
    }
    case WStype_BIN: {
      #ifdef DEBUG
      Serial.println("[DEBUG] [WS] WStype_BIN");
      #endif
      // reset ping_pong time
      lastPingPong = millis();
      Serial.print("WSB+");
      Serial.write(payload, length); Serial.println();
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
