#ifndef BTN_H
#define BTN_H

/**
 * Button debouncing
 */

/**
 * Button data structure tracking currently active state and a counter for debouncing.
 */
typedef struct {
  unsigned char active : 1;
  unsigned char count : 8 * sizeof(unsigned char) - 1;
} btn_sync;

/**
 * Poll the button's status.
 * 
 * Returns: true if button value has settled, false if it's bouncing.
 * btn->active always reflects the last settled button state.
 */
static unsigned btn_poll(unsigned pin, btn_sync* btn, unsigned limit) {
  unsigned active = digitalRead(pin);
  /* button state changes after the current status is seem 'limit' times */
  if (active != btn->active && limit < ++btn->count) {
    btn->active = active;
    btn->count = 0;
    return 1;
  } else if (active == btn->active && 0 > --btn->count) {
    btn->count = 0;
    return 1;
  }
  return 0;
}


/**
 * Button data structure tracking currently active state and a counter for debouncing.
 */
typedef struct {
  unsigned state : 1;
  unsigned long last : 8 * sizeof(unsigned)   - 1;
} btn_async;

/**
 * Notify button of change event via interrupt.
 */
static void btn_onchange(btn_async* btn) {
  if (btn->last == 0) {
    btn->last = millis();
  }
}

/**
 * Check the button's status.
 * 
 * Returns: true if button value has settled, false if it's bouncing.
 * btn->active always reflects the last settled button state.
 */
static unsigned btn_ready(unsigned pin, btn_async* btn, unsigned delay) {
  /* button state is updated after 'delay' has elapsed */
  if (millis() - btn->last > delay) {
    btn->state = digitalRead(pin);
    btn->last = 0;
    return 1;
  }
  return 0;
}

#endif