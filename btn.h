#pragma once
#ifndef BTN_H
#define BTN_H

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
 * 
 * Author: Sandro Magi <naasking@gmail.com>
 */

#include "clock.h"
#include "io.h"

/**
 * @file btn.h
 * Software debouncing for mechanical button-like inputs.
 * 
 * # Synchronous Mode
 * 
 * Declare a btn_sync variable for each synchronous button, then call btn_poll()
 * on each loop through the program. If it returns true, then the button has
 * settled and its value can be read via btn_state.active field:
 * 
 * static btn_sync btn1;
 * 
 * void loop() {
 *    if (btn_poll(PIN_X, &btn1, 7) && btn1.active == HIGH) { // assuming HIGH == pressed
 *      // button was pressed
 *    }
 * }
 * 
 * # Asynchronous Mode
 * 
 * Declare a btn_async variable for each asynchronous button and register
 * an interrupt and invoke btn_onchange() function within the interrupt. In
 * your synchronous loop, you then call btn_ready() and if it returns true
 * then the button was pressed.
 * 
 * static volatile btn_async btn1;
 * 
 * static void btn1_onchange(){
 *    btn_onchange(&btn1);
 * }
 * 
 * void setup() {
 *    attachInterrupt(digitalPinToInterrupt(PIN_X), &btn1_onchange, CHANGE);
 * }
 * void loop() {
 *    if (btn_ready(PIN_X, &btn1, 50) && btn1.state == HIGH)  { // 50 milliseconds debounce, HIGH == pressed
 *      // button was pressed
 *    }
 * }
 * 
 */

/**
 * Button data structure tracking current state and a counter for debouncing.
 */
typedef struct {
  unsigned char state : 1;
  unsigned char count : 8 * sizeof(unsigned char) - 1;
} btn_sync;

/**
 * Poll the button's status.
 * 
 * Checks whether the button has finished bouncing. btn->state always reflects the
 * last settled button state until this function returns true.
 * 
 * @param pin The pin number
 * @param btn The button state
 * @param limit The sample threshold to consider the button settled
 * @return True if button value has settled, false if it's bouncing.
 */
static unsigned btn_poll(unsigned pin, btn_sync* btn, unsigned limit) {
  unsigned state = io_readb(pin);
  /* button state changes after the current status is seem 'limit' times */
  if (state != btn->state && limit < ++btn->count) {
    btn->state = state;
    btn->count = 0;
    return 1;
  } else if (state == btn->state && 0 > --btn->count) {
    btn->count = 0;
    return 1;
  }
  return 0;
}


/**
 * Button data structure tracking current state and a counter for debouncing.
 */
typedef struct {
  unsigned state : 1;
  unsigned long last : 8 * sizeof(unsigned)   - 1;
} btn_async;

/**
 * Process button interrupt event
 * 
 * Update the button state based on an interrupt change event.
 */
static void btn_onchange(btn_async* btn) {
  if (btn->last == 0) {
    btn->last = clock_ms();
  }
}

/**
 * Check the button's status.
 * 
 * Returns: true if button value has settled, false if it's bouncing.
 * btn->state always reflects the last settled button state.
 */
static unsigned btn_ready(unsigned pin, btn_async* btn, unsigned delay) {
  /* button state is updated after 'delay' has elapsed */
  if (clock_ms() - btn->last > delay) {
    btn->state = io_readb(pin);
    btn->last = 0;
    return 1;
  }
  return 0;
}

#endif