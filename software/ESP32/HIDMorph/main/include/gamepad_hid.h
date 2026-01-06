#ifndef GAMEPAD_HID_H
#define GAMEPAD_HID_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t report_id;
    uint8_t buttons; // 8 buttons: bitmask
    int8_t x;        // X axis
    int8_t y;        // Y axis
    int8_t rx;       // optional: Right stick X
    int8_t ry;       // optional: Right stick Y
    int8_t hat;      // optional: D-pad
} __attribute__((packed)) gamepad_report_t;

typedef struct __attribute__((packed)) {
    int8_t x;
    int8_t y;
    int8_t z;
    int8_t rz;
    int8_t rx;
    int8_t ry;
    uint8_t hat;
    uint32_t buttons;
} hid_gamepad_wire_t;

void gamepad_hid_init(void);
void gamepad_hid_step(void);

#endif