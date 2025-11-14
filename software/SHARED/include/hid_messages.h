#pragma once

#include <stdint.h>

enum HID_MESSAGE_TYPE {
    DEVICE_CONNECTED,
    DEVICE_DISCONNECTED,
    NEW_REPORT
};

enum REPORT_PROTOCOL {
    PROTOCOL_NONE = 0,     ///< None
    PROTOCOL_KEYBOARD = 1, ///< Keyboard
    PROTOCOL_MOUSE = 2    ///< Mouse
};

typedef struct HID_CONNECTED {
    int dev_addr;
} HID_CONNECTED_t;

typedef struct HID_DISCONNECTED {
    int dev_addr;
} HID_DISCONNECTED_t;

typedef struct HID_NEW_REPORT {
    int dev_addr;
    int report_id;
    int protocol;
    int length;
    unsigned char data[64];
} HID_NEW_REPORT_t;

union HID_MESSAGE {
    HID_NEW_REPORT_t new_report;
    HID_CONNECTED_t connected;
    HID_DISCONNECTED_t disconnected;
};

typedef struct HID_MESSAGE_PACKET {
    char sync_word[5];
    uint16_t type;
    union HID_MESSAGE message;
} HID_MESSAGE_PACKET_t;
