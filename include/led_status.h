#pragma once

#include <Arduino.h>

#define LED_PIN 33

#define LED_OFF 0
#define LED_ON 1
#define LED_SLOW_BLINK 2
#define LED_FAST_BLINK 3
#define LED_CODE_BLINK 4

#define SLOW_BLINK_DELAY 500
#define FAST_BLINK_DELAY 100
#define CODE_PAUSE_DELAY 1000

/* Error blink codes:
 *   3 fast blinks = camera not detected (init failed / sensor not found)
 *   5 fast blinks = frame capture failure
 */
#define LED_ERR_NONE 0
#define LED_ERR_CAMERA_NOT_FOUND 3
#define LED_ERR_FRAME_CAPTURE 5

#define LED_STATUS_DISCONNECTED() ledSlowBlink()
#define LED_STATUS_CONNECTED() ledOn()
#define LED_STATUS_ERROR() ledFastBlink()
#define LED_STATUS_CODE(code) ledBlinkCode(code)

void ledBegin(uint8_t _pin = LED_PIN);
void ledOff();
void ledOn();
void ledSlowBlink();
void ledFastBlink();
void ledBlinkCode(uint8_t count);
void ledStatusHandler();
