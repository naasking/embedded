#pragma once
#ifndef CTXT_H
#define CTXT_H

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
 * Context Switching
 * 
 * This header provides an platform-agnostic context switching API.
 */

#include <setjmp.h>
#include <stdint.h>

/* context management definitions */
typedef jmp_buf ctxt;

/**
 * Switch context
 * 
 * Switches to the given context.
 * 
 * @param The context to switch to
 */
#define ctxt_switch(c) longjmp(c, 1)

/**
 * Context resume
 * 
 * Resume execution at this point, returning true if context switch occurred,
 * or false otherwise.
 * 
 * @param The context to save/resume.
 * @return 1 if control was returned via ctxt_switch(), 0 otherwise.
 */
#define ctxt_resumed(c) setjmp(c)

/**
 * Intialize context switching.
 * 
 * This initializes context switching by probing the current architecture
 * for stack direction and the locations of stack registers.
 */
static void ctxt_init()
{
	struct _ctxt_probe_data p;
	p.ref_probe = &p.probe_samePC;

	_ctxt_infer_stack_direction();

	/* do a probe with filler on stack */
	_ctxt_fill(&p);
	/* do a probe without filler */
	_ctxt_bound_low(&p);
	_infer_jmpbuf_offsets(&p);
}

/**********************************
 * Internal implementation logic. *
 **********************************/

/* the list of offsets in jmp_buf to be adjusted */
/* # of offsets cannot be greater than jmp_buf */
static int _ctxt_offsets[sizeof(jmp_buf) / sizeof(int)];
static int _ctxt_offsets_len;

/* true if stack grows up, false if down */
static int _ctxt_stack_grows_up;

/* the offset of the beginning of the stack frame in a function */
static size_t _ctxt_frame_offset;

/* This probing code is derived from Douglas Jones' user thread library */
struct _ctxt_probe_data {
	intptr_t low_bound;    /* below probe on stack */
	intptr_t probe_local;  /* local to probe on stack */
	intptr_t high_bound;   /* above probe on stack */
	intptr_t prior_local;  /* value of probe_local from earlier call */

	jmp_buf probe_env;     /* saved environment of probe */
	jmp_buf probe_sameAR;  /* second environment saved by same call */
	jmp_buf probe_samePC;  /* environment saved on previous call */

	jmp_buf * ref_probe;   /* switches between probes */
};

void_ctxt_bound_high(struct _ctxt_probe_data *p)
{
	int c;
	p->high_bound = (intptr_t)&c;
}

void _ctxt_probe(struct _ctxt_probe_data *p)
{
	int c;
	p->prior_local = p->probe_local;
	p->probe_local = (intptr_t)&c;
	_setjmp( *(p->ref_probe) );
	p->ref_probe = &p->probe_env;
	_setjmp( p->probe_sameAR );
	_ctxt_bound_high(p);
}

void _ctxt_bound_low(struct _ctxt_probe_data *p)
{
	int c;
	p->low_bound = (intptr_t)&c;
	_ctxt_probe(p);
}

void _ctxt_fill(struct _ctxt_probe_data *p)
{
	_ctxt_bound_low(p);
}

static unsigned _ctxt_infer_offsets(struct _ctxt_probe_data *pb)
{
	/* following line views jump buffer as array of long intptr_t */
	unsigned i;
	intptr_t * p = (intptr_t *)pb->probe_env;
	intptr_t * sameAR = (intptr_t *)pb->probe_sameAR;
	intptr_t * samePC = (intptr_t *)pb->probe_samePC;
	intptr_t prior_diff = pb->probe_local - pb->prior_local;
	intptr_t min_frame = pb->probe_local;

	for (i = 0; i < sizeof(jmp_buf) / sizeof(intptr_t); ++i) {
		intptr_t pi = p[i], samePCi = samePC[i];
		if (pi != samePCi) {
			if (pi != sameAR[i])
                return 1;
			if ((pi - samePCi) == prior_diff) {
				/* the i'th pointer field in jmp_buf needs to be save/restored */
				_ctxt_offsets[_ctxt_offsets_len++] = i;
				if ((_stack_grows_up && min_frame > pi) || (!_stack_grows_up && min_frame < pi)) {
					min_frame = pi;
				}
			}
		}
	}
	
	_frame_offset = (_stack_grows_up
		? pb->probe_local - min_frame
		: min_frame - pb->probe_local);
}

static void _ctxt_infer_direction_from(int *first_addr)
{
	int second;
	_stack_grows_up = (first_addr < &second);
}

static void _ctxt_infer_stack_direction()
{
	int first;
	_infer_direction_from(&first);
}

#endif