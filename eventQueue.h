#ifndef __EVENTQUEUE_H__
#define __EVENTQUEUE_H__

#include <stdint.h>
#include "mymath.h"

enum eventType_t {
    ET_MOUSE_MOVE,
    ET_MOUSE_CLICK,
    ET_KEY_UP,
    ET_KEY_DOWN
};

enum eventModifier_t {
    EM_NONE,
    EM_MOUSE_LBUTTON,
    EM_MOUSE_RBUTTON,
    EM_CONTROL,
    EM_DOWN,
    EM_UP,
    EM_LEFT,
    EM_RIGHT
};

struct event_t {
    eventType_t type;
    eventModifier_t modifier;
    v2_t cursor;
    uint32_t value;
};

struct eventManager_t {
    event_t *queue;
    uint32_t writeIndex;
    uint32_t readIndex;
    uint32_t capacity;
    uint32_t active;
};


extern event_t* PeekEvent(eventManager_t *em);
extern event_t* ReadEvent(eventManager_t *em);
extern void AddEvent(eventManager_t *em, event_t event);
#endif
