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
 * A cooperative tasking API using earliest deadline first dynamic scheduling:
 *
 *  task_run(task_sched(task_state1, task_fn1),
 *           task_sched(task_state2, task_fn2), ...)
 *
 * This won't be real-time without very careful analysis of the longest
 * code sequence between yield points, which is what determines max latency.
 *
 * Callers must define their local state as a struct and include async_state
 * and task_state:
 * 
 *  struct fn_state {
 *  	task_state;
 *      ...
 *  }
 *
 * There is only one rule to follow to ensure this all works: place all switch
 * statements in their own function. task.h uses Duff's device which does not
 * play well with switch. If you always place the switch in its own function
 * you won't have any issues.
 *
 * Cost of scheduling is O(N), where N = number of tasks. N is typically very
 * low for the contexts in which task.h should be used.
 * 
 * Every task MUST call either task_wake(), task_resched(), task_period(),
 * or task_sleep() somewhere in its processing loop in order for scheduling to
 * work correctly.
 */

#include <stdio.h>
#include "async.h"
#include "clock.h"

//FIXME: there's a problem with using await() with tasks. If a task T1 is
//selected to run then it has the earliest deadline, but if T1's await()
//condition fails it will immediately return, the scheduling loop will restart,
//and it will be picked again, fail again and return, and so on. Basically,
//T1 starves the rest of the tasks until await() succeeds. Maybe the
//behaviour of await() can be overloaded via an optional macro, and tasks
//would revise the deadline or resume condition. The overload would call
//task_sleep(1) or something, so that another task can proceed. Tasks might
//simply need their own task_await() which accepts a sleep argument.

/**
 * An async procedure
 */
typedef async task;

/**
 * Task scheduling data
 */
struct task_state {
    unsigned long deadline;     /* next deadline in ms */
    unsigned long resume; /* resume the task at the given time */
};

/**
 * Definition of task to include in your task data structure
 */
#define task_state async_state; struct task_state _task_state

/**
 * Wake the task at the given time.
 * 
 * Yields control and schedules the task to be resumed at the given
 * clock time, in milliseconds.
 * 
 * @param ms The clock time in milliseconds
 */
#define task_wake(ms) _task_state->resume = (ms); async_yield

/**
 * Sleep for the given time span.
 * 
 * Yields control and schedules the task to resume after the duration
 * in milliseconds has elapsed.
 * 
 * @param ms The duration to sleep, in milliseconds
 */
#define task_sleep(ms) task_wake(clock_ms() + ms)

/**
 * Wait for a condition to be true before proceeding.
 * @param cond The condition to wait for
 * @param ms The time to sleep if the condition fails
 */
#define task_await(cond, ms) if (!(cond)) { task_sleep(ms); }

/**
 * Reschedule the task for the given deadline.
 * 
 * Yields control and schedules the task to run before the given deadline,
 * in milliseconds.
 * 
 * @param deadline The task's new deadline
 */
#define task_resched(deadline) task_deadline(_task_state) = (deadline); async_yield

/**
 * Declare a periodic task.
 * 
 * Yields control and resets the task's next deadline according to the
 * given periodic schedule.
 * 
 * @param ms The periodic schedule, in milliseconds.
 */
#define task_period(ms) task_resched(task_deadline() + ms)

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
#define task_switch(f, t) async_call(f, t)

/**
 * Mark the beginning of a task procedure.
 * 
 * @param t The task state
 */
#define task_begin(t) struct task_state* _task_state = &(t)->_task_state; async_begin(t)

/**
 * Mark the end of a task procedure.
 */
#define task_end async_end

/**
 * Initialize a task structure.
 * 
 * Initialize the task procedure state structure.
 * 
 * @param t The task state
 */
#define task_init(t) async_init(t); task_deadline(t) = 0; (t)->_task_state.resume = 0

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
    async(*_task_f)(void*) = NULL; \
    __VA_ARGS__; \
    if (_task_ != NULL) \
        task_switch(_task_, _task_st); \
}

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
if ((f)->_task_state.resume <= _task_now && task_deadline(t) < _task_deadline) { \
  _task_deadline = (t)->_task_state.deadline; \
  _task_st = (t); \
  _task_f = (async(*)(void*))(f); \
}

#endif