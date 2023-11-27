#include "led_status.hpp"

double startMillis_led = 0;
uint8_t led_switch = 0;
uint8_t led_pin = LED_PIN;
uint8_t led_status = LED_OFF;

void led_init(uint8_t _pin) {
	led_pin = _pin;
  pinMode(led_pin, OUTPUT);  // Set LED pin as output
  digitalWrite(led_pin, HIGH);  // 1:turn off LED
}

void led_off() {
  led_status = LED_OFF;
}

void led_on() {
  led_status = LED_ON;
}

void led_slow_blink() {
  led_status = LED_SLOW_BLINK;
}

void led_fast_blink() {
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



