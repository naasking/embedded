#ifndef FSM_H
#define FSM_H

#ifndef FSM_Q_TYPE
#define FSM_Q_TYPE uint32_t
#endif
#define states(...) enum { __VA_ARGS__ }
#define events(...) enum { POLL = 0, __VA_ARGS__ }
#define FSM(states, events, ...) struct { \
    uint8_t state; \
    uint32_t volatile equeue : 24; \
    uint8_t volatile equeue_i : 8; \
    __VA_ARGS__; \
}
#define MRG(state, evt) (evt << 8) | state
#define CAT(x,y) x##_##y
#define WHEN(state, evt) CAT(state, evt): case MRG(state, evt)
#define JUMP(state, evt) goto CAT(state, evt)
#define STEP(x) uint8_t cur_evt = (x).equeue & 0xF; (x).equeue >>= 4, (x).equeue_i = max(0, --(x).equeue_i); switch(MRG((x).state, cur_evt)))
#define PENDING(x) (x).equeue
#define EVENT(x, e) noInterrupts(); (x).equeue |= e << (x).equeue_i++; interrupts()
#define NEXT(x, state) (x).state = state

#endif