#pragma once
#ifndef EVQ_H
#define EVQ_H

#include "isr.h"

//WARNING: experimental

/*
 * Event queues: asynchronous events, like interrupts, record data in an
 * event queue, which is then processed synchronously in the program loop.
 * 
 * Event queue is currently a bit vector that encodes a fixed-length enum value
 * into an unsigned long. It should be safe to use between interrupt contexts,
 * as long as the evq structure is declared volatile.
 * 
 * Add/remove ops are aware of the queue size, and will return false if
 * items cannot be added or removed because the queue is full/empty,
 * respectively. This means the client will have to implement backpressure
 * and/or drop events to proceed in an application-specific manner.
 */

//FIXME: I added queue ops as macros, but they could be static inline functions
//now that the queue type is not tied to the specific type of event.

typedef struct evq {
    unsigned long evts;
    unsigned char n;
} evq;

/**
 * The maximum number of items the evq can hold.
 * @param n The size of each individual item in bits
 * @return The maximum number of items the evq can hold
 */
#define EVQ_MAX(n) (sizeof(unsigned long) * 8 / n)

/**
 * Add an item to the event queue.
 * @param e The event queue
 * @param n The size of the event type in bits
 * @param x The event to add
 * @return True if the item was added successfully, false otherwise
 */
//#define evq_add(e, n, x) (e)->n < EVQ_MAX(n) && ((e)->evts = (e)->evts | ((x) >> n * (e)->n++), 1)
#define evq_add(e, n, x) (isr_off(), (e)->n < EVQ_MAX(n) && ((e)->evts = (e)->evts | ((x) >> n * (e)->n++), isr_on(), 1) || (isr_on(), 0))

/**
 * Remove the next item from the event queue.
 * @param e The event queue
 * @param n The size of the event type in bits
 * @param[out] x The item removed from the queue
 * @return True if an item was removed, false otherwise
 */
#define evq_pop(e, n, x) (isr_off(), (e)->n > 0 && ((x = (e)->evts & (n-1)), (e)->evts <<= n, isr_on(), 1) || (isr_on(), 0))

#endif