#include <string.h>
#include "hid_q.h"
#include "gamepad_hid.h"

#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "esp_log.h"

#include "tusb_msc_storage.h"
#include "class/msc/msc_device.h"

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

// ---------------------------------------------

const char *hid_string_descriptor[5] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
    "CedarHacks",          // 1: Manufacturer
    "ReMapper V1",         // 2: Product
    "123456",              // 3: Serials, should use chip ID
    "Game Controller HID", // 4: HID
};

const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE))};

static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 16, 10),

};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
}

void gamepad_hid_init() {

    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_string_descriptor,
        .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    int stat = tinyusb_driver_install(&tusb_cfg);

    ESP_LOGI("[HID]", "Install Gamepad HID driver status: %d\n", stat);
}

void handle_req_queue() {
    hid_evt_t e;

    int max_per_call = 16;
    int i = 0;
    for (;;) {
        if (xQueueReceive(g_hid_q, &e, 10) != pdTRUE) continue;;

        // Ensure USB is ready; skip silently if not
        if (!tud_mounted() || !tud_hid_ready()) return;

        switch (e.kind) {
        case HID_EVT_KEYBOARD:
            // TinyUSB convenience: sends report ID internally if descriptor has it
            tud_hid_keyboard_report(REPORT_ID_KBD, e.u.kbd.mods, e.u.kbd.keycodes);
            break;

        case HID_EVT_MOUSE:
            // Convenience helper for 5-byte mouse report
            tud_hid_mouse_report(REPORT_ID_MOUSE,
                                 e.u.mouse.buttons,
                                 e.u.mouse.x,
                                 e.u.mouse.y,
                                 e.u.mouse.wheel,
                                 e.u.mouse.pan);
            break;

        case HID_EVT_GAMEPAD:
            // Use generic sender for your packed struct
            tud_hid_report(REPORT_ID_GAMEPAD, &e.u.gp, sizeof(e.u.gp));
            break;
        }

        // throttle a bit
        // vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void gamepad_hid_step() {

    static float temp_x = 0;
    temp_x += 0.1;
    temp_x = temp_x >= 100 ? 0 : temp_x;

    // flush the command queue
    handle_req_queue();

    if (tud_mounted()) {
        // printf("Sending Gamepad report\n");

        gamepad_report_t report = {
            .report_id = REPORT_ID_GAMEPAD,
            .buttons = 0x01, // Button 1 pressed
            .x = temp_x,     // Move right
            .y = 0,
            .rx = 0,
            .ry = 0,
            .hat = 0 // Neutral hat
        };

        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        vTaskDelay(pdMS_TO_TICKS(100));

        // Release all
        memset(&report, 0, sizeof(report));
        report.report_id = REPORT_ID_GAMEPAD;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
    }
}