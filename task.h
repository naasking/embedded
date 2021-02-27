#pragma once
#ifndef TASK_H
#define TASK_H

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

// WARNING: experimental

/**
 * @file task.h
 * A cooperative tasking API using earliest deadline first dynamic scheduling.
 * Tasks are primarily concerned with precise timing control:
 *
 *  task_run(task_sched(task_state1, task_fn1),
 *           task_sched(task_state2, task_fn2), ...)
 *
 * This won't be real-time without very careful analysis of the longest
 * code sequence between yield points, which is what determines max latency.
 *
 * Cost of scheduling is O(N), where N = number of tasks. N should be very
 * low for the contexts in which task.h is appropriate.
 *
 * Rules for correct usage:
 * 1. Every task MUST call one of the task_X() timing functions somewhere in 
 *    its processing loop in order for scheduling to work correctly.
 * 2. Tasks must define all local state as fields in a struct.
 * 3. The task structure must include task_state somewhere there so the
 *    required scheduling data is defined:
 *      struct fn_state {
 *  	    task_state;
 *          ...
 *      }
 * 4. Place all switch statements in their own function. task.h uses Duff's
 *    device which does not play well with switch. If you always place the
 *    switch in its own function you won't have any issues.
 */

#include "async.h"
#include "clock.h"

/**
 * The task status.
 */
typedef enum TASK_STATE { TASK_START = 1, TASK_DONE = 0 } task;

/**
 * Task scheduling data.
 */
struct task_state {
    task task_k;            /* the task continuation */
    unsigned long deadline; /* next deadline in ms */
    unsigned long resume;   /* resume the task at the given time in ms */
};

/**
 * Definition of task to include in your task data structure.
 */
#define task_state struct task_state _task_state

/**
 * Yield control back to the scheduler.
 */
#define task_yield() { return __LINE__; case __LINE__: }

/**
 * Wake the task at the given time.
 * 
 * Yields control and schedules the task to be resumed at the given
 * clock time, in milliseconds.
 * 
 * @param ms The clock time in milliseconds
 */
#define task_wake(ms) {_task_state->resume = (ms); task_yield(); }

/**
 * Sleep for the given time span.
 * 
 * Yields control and schedules the task to resume after the duration
 * in milliseconds has elapsed.
 * 
 * @param ms The duration to sleep, in milliseconds
 */
#define task_sleep(ms) task_wake(clock_ms() + (ms))

/**
 * Reschedule the task for the given deadline.
 * 
 * Yields control and schedules the task to run before the given deadline,
 * in milliseconds.
 * 
 * @param deadline The task's new deadline
 */
#define task_resched(deadline) task_deadline(_task_state) = (deadline); task_yield()

/**
 * Declare a periodic task.
 * 
 * Yields control and resets the task's next deadline according to the
 * given periodic schedule.
 * 
 * @param ms The periodic schedule, in milliseconds.
 */
#define task_period(ms) task_resched(task_deadline() + (ms))

//FIXME: add a sample for exponential backoff, possibly using task resume or deadline

/**
 * The task's current deadline.
 */
#define task_deadline() _task_state->deadline

/**
 * Switch to the given task.
 * 
 * @param f The task procedure
 * @param t The task state
 */
#define task_switch(f, t) {(t)->_task_state.task_k = (f)(t);}

/**
 * Mark the beginning of a task procedure.
 * 
 * @param t The task state
 */
#define task_begin(t) struct task_state* _task_state = &(t)->task_k; switch(_task_state->task_k) { case TASK_START:

/**
 * Mark the end of a task procedure.
 */
#define task_end case TASK_DONE: task_exit; }

/**
 * Mark task as completed and exit.
 */
#define task_exit return TASK_DONE

/**
 * Initialize a task structure.
 * 
 * @param t The task state
 */
#define task_init(t) (t)->task_k = TASK_START; task_deadline(t) = 0; (t)->_task_state.resume = 0

/**
 * Run a scheduled task.
 * 
 * Selects and runs the task with earliest deadline from the
 * given list of tasks.
 * 
 * @param format The list of tasks to schedule
 */
#define task_run(...) { \
    unsigned long _task_now = clock_ms(), _task_deadline = _task_now; \
    void *_task_st = NULL; \
    task(*_task_f)(void*) = NULL; \
    __VA_ARGS__; \
    if (_task_ != NULL) \
        task_switch(_task_, _task_st); \
}

//FIXME: have conditional compilation flag for "persistent processes", which
//skips the TASK_DONE status check as all processes will run forever. Just a
//minor optimization which could save time and energy on embedded devices.

/**
 * Schedule a task.
 * 
 * Schedules a task to run, ensuring that it satisfies its wake condition.
 * Must be called within task_run().
 * 
 * @param f The task procedure
 * @param t The task state
 */
#define task_sched(f, t) \
if ((t)->task_k != TASK_DONE && (f)->_task_state.resume <= _task_now && task_deadline(t) < _task_deadline) { \
  _task_deadline = (t)->_task_state.deadline; \
  _task_st = (t); \
  _task_f = (task(*)(void*))(f); \
}

#endif