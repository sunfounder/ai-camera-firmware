#include "ws_server.h"
#include "Ticker.h"
#include "led_status.h"
#include "settings.h"
#include <ArduinoJson.h>

// #define DEBUG

void onWebSocketEvent(uint8_t cn, WStype_t type, uint8_t *payload,
                      size_t length);

WebSocketsServer *ws = nullptr;

// 存储已连接的客户端编号
uint8_t connectedClients[MAX_CLIENTS];
// 每个客户端的 ping-pong 时间戳
uint32_t clientLastPingPong[MAX_CLIENTS];
// 每个客户端的最后数据接收时间戳
uint32_t clientLastDataTime[MAX_CLIENTS];
uint8_t clientCount = 0;

String wsName = "";
String wsType = "";
String wsCheck = "SC";
String videoUrl = "";
String videoTemplate = "";

Ticker pingPongTimer;      // timer for checing ping_pong
Ticker dataTimeoutTimer;   // timer for checking data timeout
uint32_t last_pong_time = 0;
uint16_t PONG_INTERVAL = 200;
uint16_t DATA_TIMEOUT = 1000;  // 数据超时时间 (ms)
bool dataTimeoutSent = false;  // 数据超时标志位

uint32_t last_send_time = 0;
uint16_t SEND_INTERVAL = 20;

// 添加客户端到连接列表
void addClient(uint8_t cn) {
  for (int i = 0; i < clientCount; i++) {
    if (connectedClients[i] == cn) {
      return; // 已存在，不重复添加
    }
  }
  if (clientCount < MAX_CLIENTS) {
    connectedClients[clientCount] = cn;
    clientLastPingPong[clientCount] =
        millis(); // 初始化该客户端的 ping-pong 时间
    clientLastDataTime[clientCount] =
        millis(); // 初始化该客户端的数据接收时间
    clientCount++;
  }
}

// 从连接列表中移除客户端
void removeClient(uint8_t cn) {
  for (int i = 0; i < clientCount; i++) {
    if (connectedClients[i] == cn) {
      // 将最后一个元素移到被删除的位置
      connectedClients[i] = connectedClients[clientCount - 1];
      clientLastPingPong[i] = clientLastPingPong[clientCount - 1];
      clientLastDataTime[i] = clientLastDataTime[clientCount - 1];
      clientCount--;
      break;
    }
  }
}

// 更新指定客户端的 ping-pong 时间
void updateClientPingPong(uint8_t cn) {
  for (int i = 0; i < clientCount; i++) {
    if (connectedClients[i] == cn) {
      clientLastPingPong[i] = millis();
      break;
    }
  }
}

// 更新指定客户端的数据接收时间
void updateClientDataTime(uint8_t cn) {
  for (int i = 0; i < clientCount; i++) {
    if (connectedClients[i] == cn) {
      clientLastDataTime[i] = millis();
      break;
    }
  }
}

// 获取客户端数量
int WS_Server::getClientCount() { return clientCount; }

String intToString(uint8_t *value, size_t length) {
  String buf;
  for (int i = 0; i < length; i++) {
    buf += (char)value[i];
  }
  return buf;
}

void checkPingPong() {
  if (clientCount == 0) {
    return;
  }

  // 分别检查每个客户端的 ping-pong 状态
  for (int i = 0; i < clientCount; i++) {
    uint8_t cn = connectedClients[i];
    if (millis() - clientLastPingPong[i] > TIMEOUT) {
      Serial.print("[DISCONNECTED] Client ");
      Serial.print(cn);
      Serial.println(" PingPong timeout");
      if (ws != nullptr) {
        ws->disconnect(cn);
      }
      // 从列表中移除该客户端
      removeClient(cn);
      i--; // 索引回退，因为数组已变更
    }
  }
}

// 检查数据超时，超时则发送 [APPSTOP]
// 只有所有客户端都没有发数据时，才认为是暂停
void checkDataTimeout() {
  if (clientCount == 0) {
    return;
  }

  // 检查是否所有客户端都超时了
  bool allTimeout = true;
  for (int i = 0; i < clientCount; i++) {
    if (millis() - clientLastDataTime[i] <= DATA_TIMEOUT) {
      // 至少有一个客户端还在发数据
      allTimeout = false;
      break;
    }
  }

  // 只有所有客户端都超时才发送 [APPSTOP]，且只发送一次
  if (allTimeout && !dataTimeoutSent) {
    Serial.println("[APPSTOP] Data timeout from all clients");
    dataTimeoutSent = true;
  }
}

// 重置数据超时标志位
void resetDataTimeoutFlag() {
  dataTimeoutSent = false;
}

WS_Server::WS_Server() {}

void WS_Server::close() {
  if (ws != nullptr) {
    ws->close();
  }
  delay(10);
}

void WS_Server::begin(int port, String _name, String _type, String _check) {
  wsName = _name;
  wsType = _type;
  wsCheck = _check;

  // 关闭现有的WebSocket服务器
  if (ws != nullptr) {
    ws->close();
    delete ws;
    ws = nullptr;
  }

  // 创建新的WebSocket服务器实例，使用指定的端口
  ws = new WebSocketsServer(port);
  ws->begin();
  ws->onEvent(onWebSocketEvent);

  pingPongTimer.attach_ms(20, checkPingPong);
  dataTimeoutTimer.attach_ms(20, checkDataTimeout);
}

void WS_Server::loop() {
  if (ws != nullptr) {
    ws->loop();
  }
}

void WS_Server::send(String data) {
  if (ws != nullptr) {
    for (int i = 0; i < clientCount; i++) {
      ws->sendTXT(connectedClients[i], data);
    }
  }
}

// https://github.com/Links2004/arduinoWebSockets/blob/master/src/WebSocketsServer.cpp#L230
void WS_Server::sendBIN(uint8_t *payload, size_t length) {
  // bool WebSocketsServerCore::sendBIN(uint8_t num, const uint8_t * payload,
  // size_t length)
  if (ws != nullptr) {
    for (int i = 0; i < clientCount; i++) {
      ws->sendBIN(connectedClients[i], payload, length);
    }
  }
}

bool WS_Server::isConnected() { return clientCount > 0; }

void handleConfig(uint8_t client_num, String payload) {
  // Serial.println("SET+ config from websocket");
  JsonDocument config;
  JsonDocument result;
  deserializeJson(config, payload);

  result["state"] = F("ERROR");
  JsonArray errors = result["errors"].to<JsonArray>();

  // Get name
  if (config["name"].is<String>()) {
    String name = config["name"].as<String>();
    settingsSetName(name);
    result["state"] = F("OK");
  }
  if (config["type"].is<String>()) {
    String type = config["type"].as<String>();
    settingsSetType(type);
    result["state"] = F("OK");
  }
  if (config["apSsid"].is<String>()) {
    String ap_ssid = config["apSsid"].as<String>();
    settingsSetApSsid(ap_ssid);
    result["state"] = F("OK");
  }
  if (config["apPassword"].is<String>()) {
    String ap_password = config["apPassword"].as<String>();
    settingsSetApPassword(ap_password);
    result["state"] = F("OK");
  }
  if (config["staSsid"].is<String>()) {
    String staSsid = config["staSsid"].as<String>();
    if (staSsid.length() <= 0 || staSsid.length() > 32) {
      result["state"] = F("ERROR");
      errors.add(F("STA_SSID_INVALID"));
    } else {
      settingsSetStaSsid(staSsid);
      result["state"] = F("OK");
    }
  }
  if (config["staPassword"].is<String>()) {
    String staPassword = config["staPassword"].as<String>();
    if (staPassword.length() < 8 || staPassword.length() > 64) {
      result["state"] = F("ERROR");
      errors.add(F("STA_PASSWORD_INVALID"));
      return;
    } else {
      settingsSetStaPassword(staPassword);
      result["state"] = F("OK");
    }
  }
  if (config["command"].is<String>()) {
    String command = config["command"].as<String>();
    if (command == "restart-sta") {
      // Serial.println("restart-sta");
      String staSsid = settingsGetStaSsid();
      String staPassword = settingsGetStaPassword();
      if (staSsid.length() <= 0 || staSsid.length() > 32) {
        result["state"] = F("ERROR");
        errors.add(F("STA_SSID_INVALID"));
        return;
      }
      if (staPassword.length() < 8 || staPassword.length() > 64) {
        result["state"] = F("ERROR");
        errors.add(F("STA_PASSWORD_INVALID"));
        return;
      }
      bool r = wifiConnectSta(staSsid, staPassword, 5);
      if (r) {
        result["state"] = F("OK");
        result["ip"] = wifiGetStaIp();
      } else {
        result["state"] = F("ERROR");
        errors.add(F("STA_CONNECT_ERROR"));
      }
    } else if (command == "scan-wifi") {
      int count = wifiScan();
      result["state"] = F("OK");
      JsonArray networks = result["networks"].to<JsonArray>();
      for (int i = 0; i < count; i++) {
        JsonObject network = networks.add<JsonObject>();
        network["ssid"] = wifiGetScannedSSID(i);
        network["rssi"] = wifiGetScannedRSSI(i);
        network["secure"] = wifiGetScannedSecure(i);
        network["channel"] = wifiGetScannedChannel(i);
        network["bssid"] = wifiGetScannedBSSID(i);
      }
    } else if (command == "scan-clear") {
      wifiScanClean();
      result["state"] = F("OK");
    } else {
      result["state"] = F("ERROR");
      errors.add(F("UNKNOWN_COMMAND"));
    }
  }
  String result_str;
  serializeJson(result, result_str);
  if (ws != nullptr) {
    ws->sendTXT(client_num, result_str);
  }
  // Serial.println(result_str);
}

void handleSunFounderController(String payload) {
  // ------------- send simplified text -------------
  JsonDocument recvBuffer;
  deserializeJson(recvBuffer, payload);
  String result = "WS+";

  // REGIONS
  for (int i = 0; i < REGIONS_LENGTH; i++) {
    String region = String(REGIONS[i]);
    String value;
    if (recvBuffer[region].is<JsonArray>()) {
      for (int j = 0; j < recvBuffer[region].size(); j++) {
        value += recvBuffer[region][j].as<String>();
        if (j != recvBuffer[region].size() - 1)
          value += ',';
      }
    } else {
      value = recvBuffer[region].as<String>();
    }

    if (value == "true")
      value = "1";
    else if (value == "false")
      value = "0";
    if (value != "null")
      result += value;
    if (i != REGIONS_LENGTH - 1)
      result += ';';
  }

  // send
  if (millis() - last_send_time > SEND_INTERVAL) {
    Serial.println(result);
    last_send_time = millis();
  }
}

void onWebSocketEvent(uint8_t cn, WStype_t type, uint8_t *payload,
                      size_t length) {
  String out;

  switch (type) {
  // Client has disconnected
  case WStype_DISCONNECTED: {
    LED_STATUS_DISCONNECTED();
    // IPAddress remoteIp = ws.remoteIP(client_num);
    Serial.printf("[DISCONNECTED] Disconnected client[%d]\n", cn);
    // Serial.println(remoteIp.toString());
    removeClient(cn);
    break;
  }
  // New client has connected
  case WStype_CONNECTED: {
    LED_STATUS_CONNECTED();
    IPAddress remoteIp;
    if (ws != nullptr) {
      remoteIp = ws->remoteIP(cn);
    }
    Serial.printf("[CONNECTED] Connected client[%d] %s\n", cn,
                  remoteIp.toString().c_str());

    // 添加客户端到连接列表
    addClient(cn);

    // Send check_info  to client
    String check_info = String("{") + "\"Name\":\"" + wsName + "\"," +
                        "\"Type\":\"" + wsType + "\"," + "\"Check\":\"" +
                        wsCheck + "\"," + "\"video\":\"" + videoUrl + "\"," +
                        "\"StaIp\":\"" + wifiGetStaIp() + "\"," +
                        "\"VideoTemplate\":\"" + videoTemplate + "\"" + "}";
    delay(100);
    if (ws != nullptr) {
      ws->sendTXT(cn, check_info);
    }
    break;
  }
  // receive text
  case WStype_TEXT: {
#ifdef DEBUG
    Serial.println("[DEBUG] [WS] WStype_TEXT");
#endif
    // Serial.print("WStype_TEXT, length: ");Serial.println(length);

    out = intToString(payload, length);

    // 更新该客户端的 ping-pong 时间
    updateClientPingPong(cn);

    if (out.compareTo("ping") == 0) {
      // if (strcmp(out.c_str(), "ping") == 0) {
#ifdef DEBUG
        Serial.printf("[DEBUG] Received ping from client[%d]\n", cn);
#endif

      // send pong back
      uint32_t _time = millis();
      String msg = "pong " + String(_time);
      if (ws != nullptr) {
        ws->sendTXT(cn, msg);
      }
      last_pong_time = millis();
#ifdef DEBUG
      Serial.printf("[DEBUG] [WS] send PONG to [%d]\n", cn);
#endif

      return;
    }
    if (out.startsWith("SET+")) {
      handleConfig(cn, out.substring(4));
      return;
    }
    if (length > 0) {
      // 更新数据接收时间
      updateClientDataTime(cn);
      // 重置超时标志位
      resetDataTimeoutFlag();
      handleSunFounderController(out);
      return;
    }
    break;
  }
  case WStype_BIN: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_BIN from client[%d]\n", cn);
#endif
    // 更新该客户端的 ping-pong 时间
    updateClientPingPong(cn);
    // 更新数据接收时间并重置超时标志位
    updateClientDataTime(cn);
    resetDataTimeoutFlag();
    Serial.print("WSB+");
    Serial.write(payload, length);
    Serial.println();
    break;
  }
  case WStype_ERROR: {
    LED_STATUS_ERROR();
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_ERROR from client[%d]\n", cn);
#endif
    break;
  }
  case WStype_FRAGMENT_TEXT_START: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_FRAGMENT_TEXT_START from client[%d]\n",
                  cn);
#endif
    break;
  }
  case WStype_FRAGMENT_BIN_START: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_FRAGMENT_BIN_START from client[%d]\n",
                  cn);
#endif
    break;
  }
  case WStype_FRAGMENT: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_FRAGMENT from client[%d]\n", cn);
#endif
    break;
  }
  case WStype_FRAGMENT_FIN: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_FRAGMENT_FIN from client[%d]\n", cn);
#endif
    break;
  }
  case WStype_PING: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_PING from client[%d]\n", cn);
#endif
    break;
  }
  case WStype_PONG: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] WStype_PONG from client[%d]\n", cn);
#endif
    break;
  }
  default: {
#ifdef DEBUG
    Serial.printf("[DEBUG] [WS] Event Type: [%d] for client[%d]\n", type, cn);
#endif
    break;
  }
  }
}
