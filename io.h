#pragma once
#ifndef IO_H
#define IO_H

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

/**
 * @file io.h
 * Platform-agnostic I/O functions.
 */

/**
 * Set I/O mode.
 * 
 * @param pin The pin to operate on
 * @param mode The mode to use for the pin
 */
#define io_mode(pin, mode) _io_mode(pin, mode)

/**
 * Read a pin's bit.
 * 
 * @param pin The pin to read
 */
#define io_readb(pin) _io_readb(pin)

/**
 * Write a bit to a pin.
 * 
 * @param pin The pin to write
 * @param bit The bit to write
 */
#define io_writeb(pin, bit) _io_writeb(pin, bit)

/**
 * Sample an analog value.
 * 
 * @param pin The pin to sample.
 * @return The analog value sampled from the given pin.
 */
#define io_sample(pin) _io_sample(pin)

/**
 * Output a PWM signal.
 * 
 * @param pin The PWM output pin
 * @param duty The PWM duty cycle
 */
#define io_pwm(pin, duty) _io_pwm(pin, duty)


/***************** INTERNALS ******************/

//FIXME: I don't like hard-coding Arduino here, but it will do for now.
//Ultimately, these should all be define in an arch.h file which is
//selected by the build env.

/* default to Arduino API */
#ifndef _io_mode
#define _io_mode pinMode
#endif

#ifndef _io_readb
#define _io_readb digitalRead
#endif

#ifndef _io_writeb
#define _io_writeb digitalWrite
#endif

#ifndef _io_sample
#define _io_sample analogRead
#endif

#ifndef _io_pwm
#define _io_pwm analogWrite
#endif

#ifndef PWM_DUTY_MIN
#define PWM_DUTY_MIN 0
#endif

#ifndef PWM_DUTY_MAX
#define PWM_DUTY_MAX 255
#endif

#endif