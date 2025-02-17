#pragma once

#include <WebServer.h>
#include <Update.h>
#include <Preferences.h>

void settingsBegin(const char* version, int apChannel);
void settingsLoop();
