# Embedded

Header-only libraries useful for arduino and other embedded environments

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

    static btn_async btn1;

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

    static unsigned rot_state;
    static unsigned rot_pos;
    
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
        every (5, millis) {
            // do something
        }
    }

Obviously the precise timing will depend on the length of the remaining code
in the loop.