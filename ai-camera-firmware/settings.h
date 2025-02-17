#pragma once

#include <Arduino.h>

void settingsBegin(String version);
void settingsLoop();
void settingsReadConfig();
void settingsReset();

String settingsGetName();
String settingsGetType();
String settingsGetApSsid();
String settingsGetApPassword();
int settingsGetApChannel();
String settingsGetStaSsid();
String settingsGetStaPassword();
bool settingsGetCameraHorizontalMirror();
bool settingsGetCameraVerticalFlip();
int settingsGetCameraBrightness();
int settingsGetCameraContrast();
int settingsGetCameraSaturation();
int settingsGetCameraSharpness();
String settingsGetVersion();

void settingsSetName(String name);
void settingsSetType(String type);
void settingsSetApSsid(String ssid);
void settingsSetApPassword(String password);
void settingsSetApChannel(int channel);
void settingsSetStaSsid(String ssid);
void settingsSetStaPassword(String password);
void settingsSetCameraHorizontalMirror(bool mirror);
void settingsSetCameraVerticalFlip(bool flip);
void settingsSetCameraBrightness(int brightness);
void settingsSetCameraContrast(int contrast);
void settingsSetCameraSaturation(int saturation);
void settingsSetCameraSharpness(int sharpness);
void settingsSetVersion(String version);

bool settingsNameChanged();
bool settingsTypeChanged();
bool settingsApSsidChanged();
bool settingsApPasswordChanged();
bool settingsStaSsidChanged();
bool settingsStaPasswordChanged();
