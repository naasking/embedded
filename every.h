#ifndef EVERY_H
#define EVERY_H

/**
 * Defines a block of code that runs every 'x' milliseconds.
 * x: the number of time units
 * units: millis or micros
 */
#define every(x, units) \
  static unsigned long _every##_##__LINE__ = 0; \
  if (units() - _every##_##__LINE__ > (x) && (_every##_##__LINE__ = millis(),1))

#endif