#pragma once
#ifndef LED_H
#define LED_H

/*
 * Copyright 2021 Sandro Magi
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file led.h
 * Formatted LED Output
 *
 * static LedControl led;
 * static unsigned x;
 * 
 * void loop() {
 *    // 2 == position of decimal point
 *    led_uint(led, LED_ADDR, x, 2);
 * }
 */

//FIXME: should probably move this under arch/arduino/led.h or something

/* depends on LedControl arduino library */
#include <LedControl.h>

/**
 * Display a value.
 * Display an unsigned integer on an LED display.
 * @param led  The led module
 * @param addr Address of the display
 * @param x    The number to be displayed.
 * @param period The decimal point index.
 */
static void led_uint(LedControl led, unsigned addr, unsigned long x, unsigned period) {
  for (int digit = 0; digit < 8; ++digit) {
    led.setDigit(addr, digit, (byte)(x % 10), digit == period);
    x /= 10;
  }
}

#endif