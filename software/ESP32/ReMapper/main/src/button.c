#include "button.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include <string.h> // for memset if you like

// Helper: current time in microseconds (int64_t)
static inline int64_t now_us(void) { return esp_timer_get_time(); }

void init_debounce_button(Button_t *button, int GPIO_PIN) {
    // Clear struct to safe defaults
    memset(button, 0, sizeof(*button));

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // typical for button to ground
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    button->GPIO_PIN = GPIO_PIN;
    button->io_conf = io_conf;
    button->_last_change_us = now_us();

    gpio_config(&button->io_conf);

    // Initialize raw & stable states from current pin
    // Assuming active-low button: 0 => pressed
    button->_raw_down = (gpio_get_level(button->GPIO_PIN) == 0);
    button->_stable_state = button->_raw_down;
    button->is_pressed = button->_stable_state;
}

void step_debounce_button(Button_t *button) {
    // Read instantaneous raw state (active-low)
    bool raw_down = (gpio_get_level(button->GPIO_PIN) == 0);

    // Detect raw edge
    if (raw_down != button->_raw_down) {
        button->_raw_down = raw_down;
        button->_last_change_us = now_us();
        // don't change debounced state yet—wait for stability
    }

    // Has the raw level been stable for the debounce interval?
    const int64_t debounce_us = (int64_t)DEBOUNCE_TIME_MS * 1000;
    if ((now_us() - button->_last_change_us) >= debounce_us) {
        // If the debounced state differs from raw and has been stable long enough, accept it
        if (button->_stable_state != button->_raw_down) {
            bool prev = button->_stable_state;
            button->_stable_state = button->_raw_down;

            // Update public is_pressed
            button->is_pressed = button->_stable_state;

            // Click logic: a "click" == press then release (short press).
            // Arm on press; fire on release.
            if (!prev && button->_stable_state) {
                // Transition: up -> down (pressed)
                button->_armed_click = true;
            } else if (prev && !button->_stable_state) {
                // Transition: down -> up (released)
                if (button->_armed_click) {
                    button->is_clicked = true;    // one-shot; user should consume via is_clicked_debounce_button()
                    button->_armed_click = false; // disarm
                }
            }
        }
    }
}

bool is_pressed_debounce_button(Button_t *button) {
    return button->is_pressed;
}

// One-shot getter: returns true once per click, then clears flag
bool is_clicked_debounce_button(Button_t *button) {
    if (button->is_clicked) {
        button->is_clicked = false;
        return true;
    }
    return false;
}
