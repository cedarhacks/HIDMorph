#pragma once

#include "driver/gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "esp_timer.h"

#define DEBOUNCE_TIME_MS (20)

typedef struct Button {
    // configuration
    gpio_config_t io_conf;
    int GPIO_PIN;

    // public state (read-only for users)
    bool is_pressed; // debounced logical state: true while held
    bool is_clicked; // one-shot: true once on press->release

    // internal
    bool _raw_down;          // instantaneous (non-debounced) level: true if physically low (active)
    bool _stable_state;      // last debounced state
    int64_t _last_change_us; // timestamp of last raw edge
    bool _armed_click;       // became pressed, waiting for release to register click
} Button_t;

void init_debounce_button(Button_t *button, int GPIO_PIN);
void step_debounce_button(Button_t *button);
bool is_pressed_debounce_button(Button_t *button);
bool is_clicked_debounce_button(Button_t *button);