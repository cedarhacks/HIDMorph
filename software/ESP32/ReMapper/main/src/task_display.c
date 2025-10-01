#include "task_display.h"
#include <stdio.h>
#include <string.h>
#include "SSD1306_i2c_driver.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

static const char *TAG = "DISPLAY";

#define LVGL_BUF_PIXELS (SSD1306_HEIGHT * SSD1306_WIDTH)
static lv_color_t buf1[LVGL_BUF_PIXELS];
static lv_display_t *lvgl_display;
uint8_t pixels[SSD1306_HEIGHT * SSD1306_WIDTH];
SSD1306_t display;

static lv_obj_t *bar;
static lv_timer_t *anim_timer;

static void anim_cb(lv_timer_t *t) {
    static int v = 0;
    v += 3;
    if (v > 100) v = 0;
    lv_bar_set_value(bar, v, LV_ANIM_OFF);
}

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

    // // Title row
    lv_obj_t *title = lv_label_create(lv_screen_active());
    lv_label_set_text(title, "Hey CedarHacks");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_letter_space(title, 1, 0);
    lv_obj_set_pos(title, 1, 0);

    // Status dot (top-right)
    lv_obj_t *dot = lv_obj_create(lv_screen_active());
    lv_obj_set_size(dot, 4, 4);
    lv_obj_add_flag(dot, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_align(dot, LV_ALIGN_TOP_RIGHT, -1, 1);

    // Thin progress bar at the bottom
    bar = lv_bar_create(lv_screen_active());
    lv_obj_set_size(bar, 124, 6);
    lv_obj_set_style_bg_opa(bar, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 2, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);

    // make the indicator 1px taller than track for contrast
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(bar, 0, LV_PART_INDICATOR);

    // Center label (tiny)
    lv_obj_t *mid = lv_label_create(lv_screen_active());
    lv_label_set_text(mid, "Yooo is it work?");
    lv_obj_set_style_text_font(mid, &lv_font_montserrat_10, 0);
    lv_obj_center(mid);

    anim_timer = lv_timer_create(anim_cb, 200, NULL);
    lv_timer_set_user_data(anim_timer, dot);

    while (1) {
        vTaskDelay(20 / portTICK_PERIOD_MS);
        SSD1306_display(&display, pixels);
        lv_tick_inc(20);
        lv_timer_handler();
    }
}