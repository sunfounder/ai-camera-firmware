#include "led_status.h"

double startMillis_led = 0;
uint8_t led_switch = 0;
uint8_t led_pin = LED_PIN;
uint8_t led_status = LED_OFF;

/* Blink code state machine */
uint8_t led_code_count = 0;
uint8_t led_code_phase = 0;
uint32_t led_code_timer = 0;

void ledBegin(uint8_t _pin) {
  led_pin = _pin;
  pinMode(led_pin, OUTPUT);    // Set LED pin as output
  digitalWrite(led_pin, HIGH); // 1:turn off LED
}

void ledOff() { led_status = LED_OFF; }

void ledOn() { led_status = LED_ON; }

void ledSlowBlink() { led_status = LED_SLOW_BLINK; }

void ledFastBlink() { led_status = LED_FAST_BLINK; }

void ledBlinkCode(uint8_t count) {
  led_status = LED_CODE_BLINK;
  led_code_count = count;
  led_code_phase = 0;
  led_code_timer = millis();
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
    if (millis() - startMillis_led > SLOW_BLINK_DELAY) {
      startMillis_led = millis();
      led_switch = !led_switch;
      digitalWrite(led_pin, led_switch); // slow blink
    }
    break;
  case LED_FAST_BLINK:
    if (millis() - startMillis_led > FAST_BLINK_DELAY) {
      startMillis_led = millis();
      led_switch = !led_switch;
      digitalWrite(led_pin, led_switch); // fast blink
    }
    break;
  case LED_CODE_BLINK:
    {
      uint32_t now = millis();
      uint16_t duration;
      if (led_code_phase % 2 == 0) {
        // ON phase
        duration = FAST_BLINK_DELAY;
      } else if (led_code_phase == 2 * led_code_count - 1) {
        // Last OFF phase: long pause between cycles
        duration = CODE_PAUSE_DELAY;
      } else {
        // Regular OFF phase between blinks
        duration = FAST_BLINK_DELAY;
      }

      if (now - led_code_timer > duration) {
        led_code_timer = now;
        led_code_phase++;
        if (led_code_phase >= 2 * led_code_count) {
          led_code_phase = 0;
        }
      }

      // Even phase: LED ON (LOW), Odd phase: LED OFF (HIGH)
      digitalWrite(led_pin, (led_code_phase % 2 == 0) ? LOW : HIGH);
    }
    break;
  }
}
