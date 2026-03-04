#pragma once

#include <Arduino.h>

void wifiBegin();
bool wifiConnectAp(String ssid, String password, int channel = 1);
bool wifiConnectSta(String ssid, String password, uint8_t waitSecond = 0);
void wifiDisconnect();
int wifiSetHostname(String hostname);
int wifiScan();
void wifiScanClean();
String wifiGetScannedSSID(int index);
int32_t wifiGetScannedRSSI(int index);
uint8_t wifiGetScannedSecure(int index);
int32_t wifiGetScannedChannel(int index);
String wifiGetScannedBSSID(int index);
String wifiGetMacPrefix();
String wifiGetMacAddress();

String wifiGetStaIp();
String wifiGetApIp();
String wifiGetMacAddress();
String wifiGetMacPrefix();
bool wifiIsStaConnected();
