#include "task_display.h"
#include <stdio.h>
#include <string.h>
#include "SSD1306_i2c_driver.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

#define GUI_QUEUE_LEN (1)
#define LVGL_BUF_PIXELS (SSD1306_HEIGHT * SSD1306_WIDTH)

static const char *TAG = "DISPLAY";

static lv_color_t buf1[LVGL_BUF_PIXELS];
static lv_display_t *lvgl_display;
uint8_t pixels[SSD1306_HEIGHT * SSD1306_WIDTH];
SSD1306_t display;

static QueueHandle_t s_gui_q = NULL;

static inline bool color_on(lv_color_t c) {
    return lv_color_brightness(c) < 200;
}

void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    const int32_t w = lv_area_get_width(area);
    const int32_t h = lv_area_get_height(area);

    const lv_color_t *src = (const lv_color_t *)px_map;

    for (int32_t rx = 0; rx < w; rx++) {
        for (int32_t ry = 0; ry < h; ry++) {
            int i = (ry * w) + rx;
            const lv_color_t pix = src[i];

            if (color_on(pix)) {
                pixels[i] = 0xff;
            } else {
                pixels[i] = 0x00;
            }
        }
    }

    lv_display_flush_ready(disp); // Tell LVGL we are done
}

bool gui_async(gui_cb_t cb, void *arg) {
    if (!s_gui_q) return false;
    gui_msg_t m = {.cb = cb, .arg = arg, .done = NULL};
    return xQueueSend(s_gui_q, &m, 0) == pdTRUE;
}

void task_display(void *args) {
    ESP_LOGI(TAG, "TASK DISPLAY INIT\n");

    display.i2c_data_pin = 15;
    display.i2c_clock_pin = 16; // IO3 ?????
    display.i2c_freq_hz = 1000000;
    display.i2c_address = 0x3C; // ID for 32 pixel height display

    memset(pixels, 0x0, sizeof(pixels));
    SSD1306_init(&display);
    SSD1306_setup_display(&display);
    SSD1306_display(&display, pixels);

    // lvgl
    lv_init();
    // Create a display instance
    lvgl_display = lv_display_create(SSD1306_WIDTH, SSD1306_HEIGHT);
    // Set buffer(s)
    lv_display_set_buffers(lvgl_display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_FULL);
    // Set flush callback (you must implement this)
    lv_display_set_flush_cb(lvgl_display, my_flush_cb);
    lv_display_set_antialiasing(lvgl_display, false); // crisper at 1bpp style

    // s_gui_q = xQueueCreate(GUI_QUEUE_LEN, sizeof(gui_msg_t));

    // TickType_t last = xTaskGetTickCount();

    while (1) {

        // vTaskDelayUntil(&last, pdMS_TO_TICKS(20));
        vTaskDelay(20 / portTICK_PERIOD_MS); // <--- old way

        SSD1306_display(&display, pixels);
        lv_tick_inc(20);
        lv_timer_handler();

        // // drain 1 msg
        // gui_msg_t msg;
        // if (xQueueReceive(s_gui_q, &msg, 0) == pdTRUE) {
        //     if (msg.cb) {
        //         msg.cb(msg.arg);
        //     }

        //     if (msg.done) {
        //         xSemaphoreGive(msg.done); // wake the waiter
        //     }
        // }

    }
}