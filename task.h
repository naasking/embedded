#ifndef TASK_H
#define TASK_H

#ifndef TASK_MAX
#define TASK_MAX 8
#endif

#include <stdio.h>
#include "async.h"
//#include <unistd.h>

typedef struct task {
    async_state;                // the async context used to resume the process
    async (*fn)(struct task*);  // the function for the task's code
    unsigned long deadline;     // next deadline in ms
} task;

#define task_delay(t, ms) (t)->deadline = millis() + (ms); case __LINE__: return __LINE__;

task taskq[TASK_MAX];
volatile unsigned char taskc;

/*
 * Create a new task.
 */
static void task_new(async (*fn)(struct task*)) {
    unsigned char i = taskc++;
    async_init(&taskq[i]);
	taskq[i].fn = fn;
	taskq[i].deadline = 0;
}

/*
 * Mark the task completed.
 */
static void task_exit(task* t) {
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
    // use Duff's device to unroll the EDF scheduling loop
	if (taskc == 0)
		return;
    unsigned edf = 0;
    for (unsigned i = 0; i < taskc; ++i) {
        switch(taskc - i) {
        default:
            if (taskq[edf].deadline > taskq[i].deadline)
                edf = i;
            ++i;
        case 3:
            if (taskq[edf].deadline > taskq[i].deadline)
                edf = i;
            ++i;
        case 2:
            if (taskq[edf].deadline > taskq[i].deadline)
                edf = i;
            ++i;
        case 1:
            if (taskq[edf].deadline > taskq[i].deadline)
                edf = i;
            ++i;
        case 0:
            if (taskq[edf].deadline > taskq[i].deadline)
                edf = i;
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