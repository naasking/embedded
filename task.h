#ifndef TASK_H
#define TASK_H

#ifndef TASK_MAX
#define TASK_MAX 8
#endif

#include <stdio.h>
#include "async.h"

typedef struct task {
    async_state;                // the async context used to resume the process
    async (*fn)(struct task*);  // the function for the task's code
    unsigned long deadline;     // next deadline in ms
} task;

#define task_delay(t, ms) (t)->deadline = millis() + (ms); return __LINE__; case __LINE__:

task taskq[TASK_MAX];
volatile unsigned char taskc;

/*
 * Create a new task.
 */
static void task_new(async (*fn)(struct task*)) {
	//FIXME: add error checking that task count < TASK_MAX
    unsigned char i = taskc++;
    async_init(&taskq[i]);
	taskq[i].fn = fn;
	taskq[i].deadline = millis();
}

/*
 * Mark the task completed.
 */
static void task_exit(task* t) {
	// reset the task state and compact the task queue
	t->fn = NULL;
	async_init(t);
	unsigned char i = taskc;
	if (t < &taskq[i]) {
		task* last = &taskq[i];
		memcpy(t, t + sizeof(struct task), last - t - sizeof(struct task));
	}
	taskc--;
}

/*
 * Run the next task using EDF scheduling.
 *
 * NOTE: this will stop working correctly when the timer overflows.
 */
static void task_run() {
	if (taskc == 0)
		return;
	// unroll the scheduling loop using Duff's device
	unsigned edf = 0;
	unsigned long edf_d = taskq[edf].deadline - millis();
	for (unsigned i = 1; i < taskc; ++i) {
		unsigned long i_d;
		switch (taskc - i) {
		default:
			i_d = taskq[i].deadline - millis();
			if (i_d < edf_d) {
				edf = i;
				edf_d = i_d;
			}
			++i;
		case 3:
			i_d = taskq[i].deadline - millis();
			if (i_d < edf_d) {
				edf = i;
				edf_d = i_d;
			}
			++i;
		case 2:
			i_d = taskq[i].deadline - millis();
			if (i_d < edf_d) {
				edf = i;
				edf_d = i_d;
			}
			++i;
		case 1:
			i_d = taskq[i].deadline - millis();
			if (i_d < edf_d) {
				edf = i;
				edf_d = i_d;
			}
			++i;
		case 0:
			i_d = taskq[i].deadline - millis();
			if (i_d < edf_d) {
				edf = i;
				edf_d = i_d;
			}
		}
	}
	task* t = &taskq[edf];
	unsigned long d = t->deadline;
	if (d >= millis()) {
		t->_async_kcont = t->fn(&taskq[edf]);
		if (async_done(t))
			task_exit(t);
		else if (t->deadline == d)
			t->deadline = millis() + 1; // add 1ms delay if just polling
	}
}

#endif