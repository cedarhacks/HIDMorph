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

void gamepad_hid_init(void);
void gamepad_hid_step(void);

#endif