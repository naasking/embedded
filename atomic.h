#pragma once
#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * Lock-Free Atomic Operations
 * 
 * These provide some low-level operations for reading and writing values and
 * bytes without requiring locks, using only volatile read/write and only a
 * single writer. Multiple writers can possibly be supported with a spin
 * wait on _atomic_begin_write(), but this will require careful design to
 * ensure proper mutual exclusion between writers.
 * 
 * Priority is given to writers who always proceed without waiting due to the
 * single writer requirement.
 * 
 * Readers spin wait when they detect a write in progress, or that a write
 * occurred while they were reading.
 * 
 * This should also work on 8-bit microcontrollers to read multi-byte values,
 * since they also prevent torn reads.
 * 
 * It accomplishes this via a volatile 'version' state, which monotonically
 * increases with each write. Overflow shouldn't affect operations.
 */

#define atomic(T) struct atomic##_##T { \
    T value; \
    char vers; \
}

#define _atomic_read(T, version, location) \
    T x;\
    unsigned old;\
    do\
    {\
        old = _atomic_begin_read(version);\
        x = *location;\
    } while (old != _atomic_end_read(version)); \
    return x;

static
unsigned _atomic_begin_read(volatile unsigned* version) {
    unsigned v;
    // loop until version is even = no write in progress
    do
    {
        v = *version;
        if (0 == (v & 0x01)) break;   // odd version means write in progress
        //Wait(); //FIXME: should spin wait?
    } while (1);
    return v;
}

static inline
unsigned _atomic_end_read(volatile unsigned *version) {
    return *version;
}

static inline
void _atomic_begin_write(volatile unsigned *version) {
    // ++version odd: mark write in-progress
    *version |= 0x01;
}

static inline
void _atomic_end_write(volatile unsigned *version) {
    // ++version odd: mark write in-progress
    *version += 1;
}

/***************** READ OPERATIONS ********************/

/**
 * Atomically read a value.
 * @param version The version field.
 * @param location The address of the value to read.
 * @return The value at the given address.
 */
static
uint16_t atomic_readu16(volatile unsigned* version, uint16_t *location) {
    _atomic_read(uint16_t, version, location);
}

/**
 * Atomically read a value.
 * @param version The version field.
 * @param location The address of the value to read.
 * @return The value at the given address.
 */
static
int16_t atomic_readi16(volatile unsigned* version, int16_t *location) {
    _atomic_read(int16_t, version, location);
}

/**
 * Atomically read a value.
 * @param version The version field.
 * @param location The address of the value to read.
 * @return The value at the given address.
 */
static
uint32_t atomic_readu32(volatile unsigned* version, uint32_t *location) {
    _atomic_read(uint32_t, version, location);
}

/**
 * Atomically read a value.
 * @param version The version field.
 * @param location The address of the value to read.
 * @return The value at the given address.
 */
static
int32_t atomic_readi32(volatile unsigned* version, int32_t *location) {
    _atomic_read(int32_t, version, location);
}

/**
 * Atomically read a value.
 * @param version The version field.
 * @param location The address of the value to read.
 * @return The value at the given address.
 */
static
uint64_t atomic_readu64(volatile unsigned* version, uint64_t *location) {
    _atomic_read(uint64_t, version, location);
}

/**
 * Atomically read a value.
 * @param version The version field.
 * @param location The address of the value to read.
 * @return The value at the given address.
 */
static
int64_t atomic_readi64(volatile unsigned* version, int64_t *location) {
    _atomic_read(int64_t, version, location);
}

/**
 * Atomically read a byte vector.
 * @param version The version field.
 * @param output The address to which the data is copied.
 * @param location The address from which to copy.
 * @param bytes The number of bytes to copy.
 */
static
void atomic_readv(volatile unsigned* version, void* output, volatile void *location, size_t bytes) {
    unsigned old;
    do
    {
        old = _atomic_begin_read(version);
        // pretend it's const since we check for concurrent updates after the fact
        memcpy(output, (const void*)location, bytes);
    } while (old != _atomic_end_read(version));
}

/***************** WRITE OPERATIONS ********************/

/**
 * Atomically write a value.
 * @param version The version field.
 * @param location The address to write.
 * @param value The value to write.
 */
static
void atomic_writei16(volatile unsigned* version, int16_t *location, int16_t value) {
    _atomic_begin_write(version);
    *location = value;
    _atomic_end_write(version);
}

/**
 * Atomically write a value.
 * @param version The version field.
 * @param location The address to write.
 * @param value The value to write.
 */
static
void atomic_writeu16(volatile unsigned* version, uint16_t *location, uint16_t value) {
    _atomic_begin_write(version);
    *location = value;
    _atomic_end_write(version);
}

/**
 * Atomically write a value.
 * @param version The version field.
 * @param location The address to write.
 * @param value The value to write.
 */
static
void atomic_writei32(volatile unsigned* version, int32_t *location, int32_t value) {
    _atomic_begin_write(version);
    *location = value;
    _atomic_end_write(version);
}

/**
 * Atomically write a value.
 * @param version The version field.
 * @param location The address to write.
 * @param value The value to write.
 */
static
void atomic_writeu32(volatile unsigned* version, uint32_t *location, uint32_t value) {
    _atomic_begin_write(version);
    *location = value;
    _atomic_end_write(version);
}

/**
 * Atomically write a value.
 * @param version The version field.
 * @param location The address to write.
 * @param value The value to write.
 */
static
void atomic_writei64(volatile unsigned* version, int64_t *location, int64_t value) {
    _atomic_begin_write(version);
    *location = value;
    _atomic_end_write(version);
}

/**
 * Atomically write a value.
 * @param version The version field.
 * @param location The address to write.
 * @param value The value to write.
 */
static
void atomic_writeu64(volatile unsigned* version, uint64_t *location, uint64_t value) {
    _atomic_begin_write(version);
    *location = value;
    _atomic_end_write(version);
}

/**
 * Atomically write a byte vector.
 * @param version The version field.
 * @param source The address from which the data is copied.
 * @param location The location to which to copy.
 * @param bytes The number of bytes to copy.
 */
static
void atomic_writev(volatile unsigned* version, volatile void *location, const void* source, size_t bytes) {
    _atomic_begin_write(version);
    // pretend it's const since we check for concurrent updates after the fact
    memcpy((const void*)location, source, bytes);
    _atomic_end_write(version);
}

#endif