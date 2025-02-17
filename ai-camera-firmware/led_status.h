#pragma once

#include <Arduino.h>

#define LED_PIN 33

#define LED_OFF 0
#define LED_ON 1
#define LED_SLOW_BLINK 2
#define LED_FAST_BLINK 3

#define SLOW_BLINK_DELAY 500
#define FAST_BLINK_DELAY 100

#define LED_STATUS_DISCONNECTED()  ledSlowBlink()
#define LED_STATUS_CONNECTED()  ledOn()
#define LED_STATUS_ERROR()  ledFastBlink()


void ledBegin(uint8_t _pin=LED_PIN);
void ledOff();
void ledOn();
void ledSlowBlink();
void ledFastBlink();
void ledStatusHandler();


