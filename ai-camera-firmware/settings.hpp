#pragma once

#include <WebServer.h>
#include <Update.h>
#include <Preferences.h>
#include "wifi_helper.h"
#include "defaults.h"
#include "esp_camera.h"


void settingsBegin(const char* version);
void settingsLoop();

String settingsGetName();
String settingsGetType();
String settingsGetApSsid();
String settingsGetApPassword();
int settingsGetApChannel();
String settingsGetWifiSsid();
String settingsGetWifiPassword();
String settingsGetCameraHorizontalMirror();
String settingsGetCameraVerticalFlip();
String settingsGetCameraBrightness();
String settingsGetCameraContrast();
String settingsGetCameraSaturation();
String settingsGetCameraSharpness();
String settingsGetVersion();

void settingsSetName(String name);
void settingsSetType(String type);
void settingsSetApSsid(String ssid);
void settingsSetApPassword(String password);
void settingsSetApChannel(int channel);
void settingsSetWifiSsid(String ssid);
void settingsSetWifiPassword(String password);
void settingsSetCameraHorizontalMirror(String mirror);
void settingsSetCameraVerticalFlip(String flip);
void settingsSetCameraBrightness(String brightness);
void settingsSetCameraContrast(String contrast);
void settingsSetCameraSaturation(String saturation);
void settingsSetCameraSharpness(String sharpness);
void settingsSetVersion(String version);

