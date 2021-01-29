#ifndef FSM_H
#define FSM_H

// WARNING: experimental
//
// Set of macros defining an EDSL for simple finite state machines. Not
// clear if this provides any real advantage over plain event-driven C code.

// #ifndef FSM_Q_TYPE
// #define FSM_Q_TYPE uint32_t
// #endif
#define states(...) enum { __VA_ARGS__ }
#define events(...) enum { POLL = 0, __VA_ARGS__ }
#define FSM(states, events, ...) struct { \
    uint8_t state; \
    __VA_ARGS__; \
    equeue evts;
}
#define MRG(state, evt) (evt << 8) | state
#define CAT(x,y) x##_##y
#define WHEN(state, evt) CAT(state, evt): case MRG(state, evt)
#define JUMP(state, evt) goto CAT(state, evt)
#define STEP(x) switch(MRG((x).state, equeue_next((x).evts))))
#define PENDING(x) (x).equeue.i
#define EVENT(x, e) equeue_add((x).evts, e)
#define NEXT(x, state) (x).state = state

#endif