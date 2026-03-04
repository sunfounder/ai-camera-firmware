#pragma once

#include <Arduino.h>

void wifiBegin();
void wifiCheckSta();
bool wifiConnectAp(String ssid, String password, int channel = 1);
bool wifiConnectSta(String ssid, String password, bool wait = false);
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
