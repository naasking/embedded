#ifndef EVERY_H
#define EVERY_H

/**
 * Defines a block of code that runs every 'x' milliseconds.
 */
#define every(x) static unsigned long _every##_##__LINE__ = 0; if (millis() - _every##_##__LINE__ > (x))

#endif