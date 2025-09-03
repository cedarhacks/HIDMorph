#pragma once

#include "hid_messages.h"

typedef enum EventType {
    EVENT_KEY_PRESSED,
    EVENT_KEY_RELEASED
} EventType_t;

typedef struct Event {
    EventType_t type;
    uint8_t keycode;
    char ascii;
} Event_t;
