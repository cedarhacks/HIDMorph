#include "hid_q.h"
#include "esp_log.h"

QueueHandle_t output_events_q = NULL;
QueueHandle_t input_events_q = NULL;
static const char *TAG = "HID_QUEUE";

void hid_queue_init(QueueHandle_t *q) {
    (*q) = xQueueCreate(16, sizeof(hid_evt_t));
    if (!(*q))
        ESP_LOGE(TAG, "Failed to create HID queue!");
}

bool hid_post_keyboard(QueueHandle_t *q, uint8_t mods, const uint8_t keycodes[6], bool is_released, TickType_t to) {
    hid_evt_t e = {.kind = HID_EVT_KEYBOARD,};
    e.u.kbd.mods = mods;
    e.u.kbd.is_released = is_released;
    
    for (int i = 0; i < 6; ++i)
        e.u.kbd.keycodes[i] = keycodes ? keycodes[i] : 0;
    return xQueueSend((*q), &e, to) == pdTRUE;
}

bool hid_post_mouse(QueueHandle_t *q, uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan, TickType_t to) {
    hid_evt_t e = {.kind = HID_EVT_MOUSE,
                   .u.mouse = {.buttons = buttons, .x = dx, .y = dy, .wheel = wheel, .pan = pan}};
    return xQueueSend((*q), &e, to) == pdTRUE;
}

bool hid_post_gamepad(QueueHandle_t *q, uint16_t buttons, int8_t x, int8_t y, int8_t rx, int8_t ry, uint8_t hat, TickType_t to) {
    hid_evt_t e = {.kind = HID_EVT_GAMEPAD};
    e.u.gp.report_id = REPORT_ID_GAMEPAD;
    e.u.gp.buttons = buttons;
    e.u.gp.x = x;
    e.u.gp.y = y;
    e.u.gp.rx = rx;
    e.u.gp.ry = ry;
    e.u.gp.hat = hat;
    return xQueueSend((*q), &e, to) == pdTRUE;
}
