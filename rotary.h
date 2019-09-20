#ifndef __ROTARY_H__
#define __ROTARY_H__

/**
 * Rotary encoder debouncing
 */

enum ROTARY_TRANSITIONS {
  ROTARY_CW  = 10260, //=B0010100000010100,
  ROTARY_CCW = 16770, //=B0100000110000010,
};

#define rotary_cw(r) (1 << (r)) & ROTARY_CW
#define rotary_ccw(r) (1 << (r)) & ROTARY_CCW

/**
 * Process a rotary encoder step
 *
 * Returns:
 *  1 = 1 step clockwise
 * -1 = 1 step counter clockwise
 *  0 = no step (either no movement or invalid due to bouncing)
 */
static int8_t rotary_step(uint8_t *rotary, uint8_t rotb, uint8_t rota) {
  *rotary = 0x0f & (*rotary << 2) | digitalRead(rotb) << 1 | digitalRead(rota);
  return rotary_cw(*rotary) ? 1:
         rotary_ccw(*rotary)?-1:
                              0;
}

#endif