#pragma once

#include <WiFi.h>
#include <ESPmDNS.h>       // mDNS

#include "log.h"

void wifiBegin();
void wifiCheckSta();
bool wifiConnectAp(String ssid, String password, int channel = 1); 
bool wifiConnectSta(String ssid, String password);
int wifiSetHostname(String hostname);
uint8_t wifiScan();
void wifiScanClean();
String wifiGetScannedSSID(uint8_t index);
int32_t wifiGetScannedRSSI(uint8_t index);
uint8_t wifiGetScannedSecure(uint8_t index);
int32_t wifiGetScannedChannel(uint8_t index);
String wifiGetScannedBSSID(uint8_t index);
String wifiGetMacPrefix();
String wifiGetMacAddress();

String wifiGetStaIp();
String wifiGetApIp();
String wifiGetMacAddress();
String wifiGetMacPrefix();
bool wifiIsStaConnected();
bool wifiIsConnected();
