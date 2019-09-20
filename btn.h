#ifndef __BTN_H__
#define __BTN_H__

/**
 * Button debouncing
 */

typedef struct {
  unsigned active : 1;
  unsigned count : 8 * sizeof(char) - 1;
} btn_state;

/**
 * Debounce button.
 * 
 * Returns true if button is currently bouncing, false otherwise.
 */
static bool btn_bouncing(unsigned pin, btn_state* btn) {
  unsigned active = digitalRead(pin);
  if (active != btn->active && 7 < ++btn->count) {
    // count 7 consecutive changes before considering the button pressed
    btn->active = active;
    btn->count = 0;
    return false;
  } else if (active == btn->active && 0 > --btn->count) {
    btn->count = 0;
    return false;
  }
  return true;
}

#endif