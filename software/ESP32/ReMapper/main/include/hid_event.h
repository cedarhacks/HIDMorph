#pragma once

#include "hid_messages.h"

typedef enum EventType {
    EVENT_KEY_PRESSED,
    EVENT_KEY_RELEASED,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_BUTTON_PRESS,
    EVENT_MOUSE_BUTTON_RELEASE,
    EVENT_MOUSE_WHEEL,
} EventType_t;

typedef struct Event {
    EventType_t type;
    uint8_t keycode;
    char ascii;
    int mouse_button;
    int mouse_dx;
    int mouse_dy;
    int mouse_wheel;
} Event_t;
