#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "tusb.h"

// report IDs (must match your descriptor)
#define REPORT_ID_KBD     0x01
#define REPORT_ID_MOUSE   0x02
#define REPORT_ID_GAMEPAD 0x03

typedef enum {
    HID_EVT_KEYBOARD,
    HID_EVT_MOUSE,
    HID_EVT_GAMEPAD
} hid_evt_kind_t;

// --- structs ---
typedef struct {
    uint8_t mods;
    uint8_t keycodes[6];
} hid_kbd_t;

typedef struct {
    uint8_t buttons;
    int8_t x, y, wheel, pan;
} hid_mouse_t;

typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint16_t buttons;
    int8_t x, y, rx, ry;
    uint8_t hat;
} hid_gamepad_t;

typedef struct {
    hid_evt_kind_t kind;
    union {
        hid_kbd_t     kbd;
        hid_mouse_t   mouse;
        hid_gamepad_t gp;
    } u;
} hid_evt_t;

// global queue handle
extern QueueHandle_t output_events_q;
extern QueueHandle_t input_events_q;

// API
void hid_queue_init(QueueHandle_t* q);
bool hid_post_keyboard(QueueHandle_t* q, uint8_t mods, const uint8_t keycodes[6], TickType_t to);
bool hid_post_mouse(QueueHandle_t* q, uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan, TickType_t to);
bool hid_post_gamepad(QueueHandle_t* q, uint16_t buttons, int8_t x, int8_t y, int8_t rx, int8_t ry, uint8_t hat, TickType_t to);
