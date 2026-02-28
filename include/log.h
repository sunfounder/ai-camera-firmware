#pragma once

#include <Arduino.h>

#include "defaults.h"

void debug(String msg);
void info(String msg);
void error(String msg);
void debug(String msg, String data);
void info(String msg, String data);
void error(String msg, String data);
