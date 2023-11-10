#pragma once

#include <Arduino.h>

#define LED_PIN 33

#define LED_OFF 0
#define LED_ON 1
#define LED_SLOW_BLINK 2
#define LED_FAST_BLINK 3

#define SLOW_BLINK_DELAY 500
#define FAST_BLINK_DELAY 100

#define LED_STATUS_DISCONNECTED()  led_slow_blink()
#define LED_STATUS_CONNECTED()  led_on()
#define LED_STATUS_ERROOR()  led_fast_blink()


void led_init(uint8_t _pin=LED_PIN);
void led_off();
void led_on();
void led_slow_blink();
void led_fast_blink();
void led_status_handler();


