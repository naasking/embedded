#ifndef __BTN_H__
#define __BTN_H__

/**
 * Button debouncing
 */

#ifndef BTN_LIMIT
#define BTN_LIMIT 7
#endif

/**
 * Button data structure tracking currently active state and a counter for debouncing.
 */
typedef struct {
  unsigned active : 1;
  unsigned count : 8 * sizeof(char) - 1;
} btn_state;

/**
 * Debounce button.
 * 
 * Returns: true if button is currently bouncing, false otherwise.
 * btn->active always reflects the last settled button state.
 */
static unsigned btn_bouncing(unsigned pin, btn_state* btn) {
  unsigned active = digitalRead(pin);
  if (active != btn->active && BTN_LIMIT < ++btn->count) {
    /* count 7 consecutive changes before considering the button pressed */
    btn->active = active;
    btn->count = 0;
    return 0;
  } else if (active == btn->active && 0 > --btn->count) {
    btn->count = 0;
    return 0;
  }
  return 1;
}

#endif