#include "led_status.h"

static double startMillis_led = 0;
static uint8_t led_switch = 0;
static uint8_t led_pin = LED_PIN;

// Two-layer state
static uint8_t led_state = LED_OFF;
static uint8_t led_error_flags = LED_ERR_NONE;

// Blink-code state machine (for showing errors)
static uint8_t led_code_phase = 0;
static uint8_t led_code_current_blinks = 0;
static uint32_t led_code_timer = 0;

// Active error index being displayed (cycles through set bits)
static uint8_t led_code_error_index = 0;

void ledBegin(uint8_t _pin) {
  led_pin = _pin;
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH); // 1 = off (active-low)
}

void ledOff() { led_state = LED_OFF; }
void ledOn() { led_state = LED_ON; }
void ledSlowBlink() { led_state = LED_SLOW_BLINK; }
void ledFastBlink() { led_state = LED_FAST_BLINK; }

// ---- Two-layer API ----

void ledSetState(uint8_t state) {
  led_state = state;
}

void ledSetError(uint8_t error) {
  led_error_flags |= error;
  // Restart blink-code machine for the new error set
  led_code_phase = 0;
  led_code_timer = millis();
}

void ledClearError(uint8_t error) {
  led_error_flags &= ~error;
  if (led_error_flags == 0) {
    led_code_phase = 0; // reset blink machine
  }
}

uint8_t ledGetErrors() {
  return led_error_flags;
}

// Map error flag to blink count
static uint8_t errorToBlinks(uint8_t flag) {
  switch (flag) {
    case LED_ERR_CAMERA_NOT_FOUND: return LED_ERR_BLINKS_CAMERA_NOT_FOUND;
    case LED_ERR_FRAME_CAPTURE:    return LED_ERR_BLINKS_FRAME_CAPTURE;
    default: return 3;
  }
}

// Get next active error flag (cyclically)
static uint8_t nextErrorFlag() {
  if (led_error_flags == 0) return 0;
  // Cycle through bits 0..7
  for (uint8_t i = 0; i < 8; i++) {
    led_code_error_index = (led_code_error_index + 1) & 0x07;
    uint8_t flag = 1 << led_code_error_index;
    if (led_error_flags & flag) return flag;
  }
  return 0; // shouldn't happen
}

// ---- Common blink-code state machine ----
// Drives LED through: (ON brief) (OFF brief) × N → (long OFF) → repeat
// When multiple errors, cycles through them.

static void runBlinkCode() {
  uint32_t now = millis();
  uint16_t duration;

  if (led_code_phase % 2 == 0) {
    // ON phase
    duration = FAST_BLINK_DELAY;
  } else if (led_code_phase == 2 * led_code_current_blinks - 1) {
    // Last OFF phase: long pause, then advance to next error
    duration = CODE_PAUSE_DELAY;
  } else {
    // Regular OFF phase between blinks
    duration = FAST_BLINK_DELAY;
  }

  if (now - led_code_timer > duration) {
    led_code_timer = now;
    led_code_phase++;

    if (led_code_phase >= 2 * led_code_current_blinks) {
      // Finished this error's blink cycle → move to next error
      led_code_phase = 0;
      uint8_t next = nextErrorFlag();
      led_code_current_blinks = errorToBlinks(next);
    }
  }

  // Even phase: LED ON (LOW for active-low), Odd: OFF (HIGH)
  digitalWrite(led_pin, (led_code_phase % 2 == 0) ? LOW : HIGH);
}

// ---- Main handler ----

void ledStatusHandler() {
  // Priority: errors → blink codes; no errors → show led_state
  if (led_error_flags != LED_ERR_NONE) {
    // Initialize blink state if needed
    if (led_code_current_blinks == 0) {
      uint8_t first = nextErrorFlag();
      led_code_current_blinks = errorToBlinks(first);
      led_code_phase = 0;
      led_code_timer = millis();
    }
    runBlinkCode();
    return;
  }

  // No errors: show connection state
  switch (led_state) {
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
      digitalWrite(led_pin, led_switch);
    }
    break;
  case LED_FAST_BLINK:
    if (millis() - startMillis_led > FAST_BLINK_DELAY) {
      startMillis_led = millis();
      led_switch = !led_switch;
      digitalWrite(led_pin, led_switch);
    }
    break;
  }
}
