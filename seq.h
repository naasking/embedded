#pragma once
#ifndef SEQ_H
#define SEQ_H

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
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLISEQCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Pulled from: https://github.com/naasking/gen.h
 * 
 * Author: Sandro Magi <naasking@gmail.com>
 */

/**
 * @file seq.h
 * Stackless Generators
 *
 * Taking inspiration from protothreads and C# generators,
 * this is a generator implementation for C based on Duff's device.
 *
 * Features:
 *
 * 1. Subroutines can have persistent state that isn't just static state, because
 *    each generator subroutine accepts its own struct it uses as a parameter, and
 *    the generator state is stored there.
 * 2. Because of the more flexible state handling, generator subroutines can be
 *    nested in tree-like fashion which permits fork/join concurrency patterns.
 *
 * Caveats:
 *
 * 1. Due to compile-time bug, MSVC requires changing: 
 *      Project Properties > Configuration Properties > C/C++ > General > Debug Information Format
 *    from "Program Database for Edit And Continue" to "Program Database".
 * 2. As with protothreads, you have to be very careful with switch statements within an generator
 *    subroutine. Rule of thumb: just place all switch statements in their own function.
 * 3. As with protothreads, you can't make blocking system calls and preserve the generator semantics.
 *    These must be changed into non-blocking calls that test a condition.
 * 4. Each generator subroutine must declare a second parameter which is a pointer
 *    that's used to return the next result.
 * 5. Always wrap blocks in {} because they
 */

#include <stdlib.h>
#include <malloc.h>

/**
 * The generator status.
 */
typedef enum SEQ_STATUS { SEQ_DONE = 0, SEQ_INIT = 1 } generator;

/**
 * Declare the generator state.
 */
#define seq_state generator _seq_k

/**
 * Mark the start of an generator.
 *
 * @param k The sequence state.
 */
#define seq_begin(k) switch((k)->_seq_k) { default:

/**
 * Mark the end of a generator subroutine.
 */
#define seq_end return SEQ_DONE; }

/**
 * Yield execution.
 * @param x The returned value.
 */
#define yield(x) { *out=(x); return (generator)__LINE__; case __LINE__: }

/**
 * Exit the current generator.
 */
#define seq_exit return SEQ_DONE

/**
 * Initialize a new generatorator.
 * @param state The generator procedure state to initialize.
 */
#define seq_init(state) ((state)->_seq_k=SEQ_INIT)

/**
 * Check if generatorator is done.
 * @param state The generator procedure state to check.
 */
#define seq_done(state) ((state)->_seq_k==SEQ_DONE)

/**
 * Call a generator.
 *
 * Calling the function itself will return true if the generator call is complete,
 * or false if it's still in progress.
 * @param f The generator procedure.
 * @param state The generator procedure state.
 */
#define seq_next(f, state, out) (seq_done(state) ? SEQ_DONE : (generator)(((state)->_seq_k=(f)(state, out))))


// Example:
typedef struct foo { seq_state; int salary; } * foo_t;

static
generator bonus(foo_t self, int* out) {
    seq_begin(self);
    yield(3);
    if (self->salary > 30000)
        yield(99);
    seq_end;
}

/**
 * The function type for a dynamic sequence.
 */
typedef generator (*dseq_fn)(dseq_state*, void*);

typedef struct {
    seq_state;
} dseq_state;

/**
 * A dynamically dispatched sequence.
 */
typedef struct {
    dseq_fn fn;
    dseq_state* state;
} dseq;

/**
 * Create a dynamic sequence.
 * @param fn The generator function.
 * @param state The generator state.
 * @param dseq The initialized dynamic sequence.
 */
static inline
void dseq_init(dseq_fn fn, dseq_state* state, dseq* out) {
    out->fn = fn;
    out->state = state;
}

/**
 * Obtain the next element from a dynamic sequence.
 * @param self The generator state.
 * @param out The next value returned.
 * @return The generator state.
 */
static inline
generator dseq_next(dseq* self, void* out) {
    return seq_next(self->fn, self->state, out);
}

struct seq_map {
    dseq seq;
    size_t sz_tmp;
    void* (*map)(void*);
};

/**
 * Create a dynamic map sequence from a given static sequence.
 * @param fn The function whose values will be mapped.
 * @param state The function state.
 * @param map The mapping function.
 * @param sz_tmp The size of the values returned by 'fn'.
 * @param out The map structure to initialize.
 */
static inline
void map_seq(generator (*fn)(void*, void*), dseq_state* state, void* (*map)(void*), size_t sz_tmp, struct seq_map* out) {
    out->map = map;
    out->sz_tmp = sz_tmp;
    dseq_init((dseq_fn)&map_next, state, &out->seq);
}

/**
 * Create a dynamic map sequence from a given dynamic sequence.
 * @param seq The sequence to map.
 * @param map The mapping function.
 * @param sz_tmp The size of the values returned by 'seq'.
 * @param out The map structure to initialize.
 */
static inline
void map_dseq(dseq* seq, void* (*map)(void*), size_t sz_tmp, struct seq_map* out) {
    out->map = map;
    out->sz_tmp = sz_tmp;
    dseq_init(seq->fn, seq->state, &out->seq);
}

/**
 * Map the next element in the sequence.
 * @param self The mapped sequence.
 * @param out The map structure to initialize.
 * @return The generator state.
 */
static inline
generator map_next(struct seq_map* self, void* out) {
    void* tmp = alloca(self->sz_tmp);
    generator x = dseq_next(&self->seq, tmp);
    out = self->map(tmp); // map temporary to final output
    return x;
}

#endif  