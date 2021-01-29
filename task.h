#ifndef TASK_H
#define TASK_H

// WARNING: experimental
//
// A simple cooperative tasking API using earliest deadline first scheduling:
//
//		task_sched(task_run(task_state1, task_fn1),
//					task_run(task_state2, task_fn2), ...)
//
// This won't be real-time unless very careful analysis is made concerning the
// longest code sequence to ascertain max latency.
//
// There might be a way to add real-time tasks via a timer though. The idea
// is to have two "kernels", this cooperative one and an interrupt-driven
// real-time mode, and switch between them with setjmp/longjmp.

#include <stdio.h>
#include "async.h"

enum TASK_STATE { RUNNING, WAITING, }

typedef struct task_state {
    async_state;                // the async context used to resume the process
    unsigned long deadline;     // next deadline in ms
} task_state;

// delay the task until after the deadline 
//FIXME: this isn't quite correct. delay() should not set a deadline, but should
//pause the task until the delay expires. yield() should accept a deadline. May
//need a single clock/time field for use as a deadline or resumption time, with a
//task_state field to discriminate.
#define task_delay(t, ms) (t)->deadline = millis() + (ms); async_yield
#define task_yield(t, deadline) (t)->deadline = millis() + (ms); async_yield

/* find and run task with earliest deadline */
#define task_sched(...) { \
	unsigned __deadline = millis(); \
	struct async *__st = NULL; \
	async(*__f)(struct async*) = NULL; \
	__VA_ARGS__; \
	if (__f != NULL) \
		async_call(__f, __st); \
}
#define task_run(st, t) if ((t)->deadline <= __deadline) { __deadline = (t)->deadline; __st = (struct async*)(st); __f = (async(*)(async*))(t); }

#endif