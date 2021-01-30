# Embedded

Header-only libraries useful for Arduino and other embedded environments

## btn.h

Abstraction for software button debouncing:

### Synchronous Debouncing

This must be invoked regularly in the main program loop:
    
    static btn_sync btn1;

    void loop() {
        // assuming HIGH==pressed, 7 = number of comparisons, compare btn1.active to LOW if using internal pullup
        if (btn_poll(PIN_X, &btn1, 7) && btn1.state == HIGH) {
            // button was pressed
        }
    }
    
### Asynchronous Button Debouncing

This requires an interrupt handler and a synchronous check in the main program loop:

    static volatile btn_async btn1;

    static void btn1_onchange() {
        btn_onchange(&btn1);
    }
    
    void setup() {
        attachInterrupt(digitalPinToInterrupt(PIN_X), &btn1_onchange, CHANGE);
    }

    void loop() {
        // 50 milliseconds debounce, HIGH == pressed
        if (btn_ready(PIN_X, &btn1, 50) && btn1.state == HIGH)  {
            // button was pressed
        }
    }

## rotary.h

Software debouncing for quadrature rotary encoders. Process encoder events
in an interrupt handler so you don't miss steps:

    static volatile unsigned rot_state;
    static volatile unsigned rot_pos;
    
    void setup() {
        attachInterrupt(digitalPinToInterrupt(PIN_X), &rot_onchange, CHANGE);
    }

    static void rot_onchange() {
        rot_pos += rotary_step(&rot_state, PIN_A, PIN_B);
    }

## led.h

Print a number to an LED display using the LedControl library:

    static LedControl led;
    static unsigned x;

    void loop() {
        // 2 == position of decimal point
        led_uint(led, LED_ADDR, x, 2);
    }

## every.h

A convenient macro for synchronously running some code periodically:

    void loop() {
        every(5, millis) {
            // do something
        }
    }

Obviously the precise timing will depend on the length of the remaining code
in the loop.

## async.h

Lightweight, stackless async/await pattern:

    #include "async.h"

    struct async pt;
    struct timer timer;

    async example(struct async *pt) {
        async_begin(pt);
        
        while(1) {
            if(initiate_io()) {
                timer_start(&timer);
                await(io_completed() || timer_expired(&timer));
                read_data();
            }
        }
        async_end;
    }

## clock.h

Abstract over clock API:

Function|Description
--------|-----------
*clock_ms()*|The clock time in milliseconds
*clock_us()*|The clock time in microseconds

## io.h

Abstract over general purpose I/O API:

Function|Description
--------|-----------
*io_mode(pin, mode)*|Set the pin's mode
*io_readb(pin)*|Read a bit from the pin
*io_writeb(pin, bit)*|Write a bit to the pin
*io_sample(pin)*|Read an analog value from the pin
*io_pwm(pin, duty)*|Output a PWM signal to a pin with a given duty cycle


# Experimental

## task.h

Lightweight cooperative, stackless tasking API featuring dynamic, earliest deadline
first scheduling. Tasks are mainly concerned with precise timing control:

    #include "task.h"

    // sizeof(task_state) == sizeof(unsigned) + 2 * sizeof(unsigned long)
    typedef struct { task_state; ... } t_state;

    static t_state t1state;
    static t_state t2state;

    task example(t1state *s) {
        task_begin(s);
        
        while(1) {
            ...
            // check a condition every 10 ms until it fails
            while(condition)
                task_sleep(10);

            // sleep and resume task 80 ms after its last deadline
            task_wake(task_deadline(s) + 80);
            ...
            // sets task next deadline to 200 ms after its last deadline
            // essentially running the task every 200 ms. Equivalent to:
            //task_resched(task_deadline(s) + 200);
            task_period(200);
        }
        task_end;
    }

    void setup() {
        task_init(&t1state);
        task_init(&t2state);
    }

    void loop() {
        task_run(task_sched(example, &t1state),
                 task_sched(example, &t2state));
    }

This uses Duff's device, like async.h, and so local variables all need to
be placed in the task state structure, and you should be careful with
the use of switch statements. A simple rule of thumb that avoids all
difficulties: place all switch statements in their own functions.

## ctxt.h

A platform-agnostic context switching API that enables true stack/context
switching with which you can create proper cooperative threading, fibers,
coroutines, etc.