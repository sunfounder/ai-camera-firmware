#pragma once

#include <Arduino.h>

#define LED_PIN 33

/* ---- LED display modes (for led_state) ---- */
#define LED_OFF 0
#define LED_ON 1
#define LED_SLOW_BLINK 2
#define LED_FAST_BLINK 3

/* ---- Timings ---- */
#define SLOW_BLINK_DELAY 500
#define FAST_BLINK_DELAY 100
#define CODE_PAUSE_DELAY 1000

/* ---- Error flags (bitmask, independent of led_state) ---- */
#define LED_ERR_NONE             0x00
#define LED_ERR_CAMERA_NOT_FOUND 0x01
#define LED_ERR_FRAME_CAPTURE    0x02

/* Blink counts per error flag */
#define LED_ERR_BLINKS_CAMERA_NOT_FOUND 3
#define LED_ERR_BLINKS_FRAME_CAPTURE    5

/* ---- Convenience macros (backward compatible) ---- */
#define LED_STATUS_DISCONNECTED() ledSetState(LED_SLOW_BLINK)
#define LED_STATUS_CONNECTED()    ledSetState(LED_ON)
#define LED_STATUS_ERROR()        ledSetState(LED_FAST_BLINK)
#define LED_STATUS_CODE(code)     ledSetError(code)
#define LED_CLEAR_CODE(code)      ledClearError(code)

/* ---- API ---- */
void ledBegin(uint8_t _pin = LED_PIN);
void ledOff();
void ledOn();
void ledSlowBlink();
void ledFastBlink();

// New two-layer API
void ledSetState(uint8_t state);
void ledSetError(uint8_t error);
void ledClearError(uint8_t error);
uint8_t ledGetErrors();

void ledStatusHandler();
