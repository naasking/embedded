#ifndef __LED_H__
#define __LED_H__

/* depends on LedControl library */
#include <LedControl.h>

/**
 * Display an unsigned integer on an LED display
 *  addr     address of the display
 *  x        the number to be displayed.
 *  period   the decimal point index.
 */
static void led_uint(LedControl led, unsigned addr, unsigned long x, unsigned period) {
  for (int digit = 0; digit < 8; ++digit) {
    led.setDigit(addr, digit, (byte)(x % 10), digit == period);
    x /= 10;
  }
}

#endif
