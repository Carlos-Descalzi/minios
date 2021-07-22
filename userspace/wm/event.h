#ifndef _EVENT_H_
#define _EVENT_H_

#include "stdint.h"
#include "common.h"

typedef enum {
    EVENT_TYPE_NONE,
    EVENT_TYPE_MOUSE,
    EVENT_TYPE_KEY
} EventType;

typedef enum {
    MOUSE_EVENT_PRESS,
    MOUSE_EVENT_RELEASE,
    MOUSE_EVENT_MOVE,
    MOUSE_EVENT_DRAG
} MouseEventType;

typedef struct {
    MouseEventType type;
    Point pos;
    int button;
} MouseEvent;

typedef enum {
    KEY_EVENT_PRESS,
    KEY_EVENT_RELEASE
} KeyEventType;

typedef struct {
    KeyEventType type;
    int key_code;
} KeyEvent;

typedef struct {
    EventType type;
    union {
        MouseEvent mouse;
        KeyEvent key;
    };
} Event;


#endif
