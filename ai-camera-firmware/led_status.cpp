#include "led_status.h"

double startMillis_led = 0;
uint8_t led_switch = 0;
uint8_t led_pin = LED_PIN;
uint8_t led_status = LED_OFF;

void ledBegin(uint8_t _pin) {
	led_pin = _pin;
  pinMode(led_pin, OUTPUT);  // Set LED pin as output
  digitalWrite(led_pin, HIGH);  // 1:turn off LED
}

void ledOff() {
  led_status = LED_OFF;
}

void ledOn() {
  led_status = LED_ON;
}

void ledSlowBlink() {
  led_status = LED_SLOW_BLINK;
}

void ledFastBlink() {
  led_status = LED_FAST_BLINK;
}

void ledStatusHandler() {
  switch (led_status) {
    case LED_OFF:
      digitalWrite(led_pin, HIGH);
      break;
		case LED_ON:
			digitalWrite(led_pin, LOW);
			break;
    case LED_SLOW_BLINK:
      if (millis() - startMillis_led > SLOW_BLINK_DELAY){
        startMillis_led = millis();
        led_switch = !led_switch;
        digitalWrite(led_pin, led_switch); // slow blink
      }
      break;
    case LED_FAST_BLINK:
      if (millis() - startMillis_led > FAST_BLINK_DELAY){
        startMillis_led = millis();
        led_switch = !led_switch;
        digitalWrite(led_pin, led_switch); // fast blink
      }
  }
}



