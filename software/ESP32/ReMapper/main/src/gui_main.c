#include "gui_main.h"
#include "esp_log.h"
#include "task_display.h"
#include <stdio.h>

#define BUTTON_SIZE (DISPLAY_H - 10)

lv_obj_t *pager;              // horizontal scroller
static int current_index = 0; // which file is “selected”
static int file_count = 5;

static const char *files[] = {
    "boot.log",
    "really_long_filename_that_needs_scrolling.txt",
    "levels/map01.json",
    "screenshot_0001.bmp",
    "notes.md"};

// Jump to an index (clamped), animate scroll
static void pager_goto(int idx, lv_anim_enable_t anim) {
    if (file_count == 0) return;
    if (idx < 0) idx = 0;
    if (idx >= file_count) idx = file_count - 1;
    current_index = idx;

    lv_obj_t *item = lv_obj_get_child(pager, idx);
    if (item) lv_obj_scroll_to_view(item, anim);
}

static void on_left(lv_event_t *e) {
    pager_goto(current_index - 1, LV_ANIM_ON);
}
static void on_right(lv_event_t *e) {
    pager_goto(current_index + 1, LV_ANIM_ON);
}

void create_file_pager(lv_obj_t *parent) {
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);

    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Left arrow
    lv_obj_t *button_l = lv_btn_create(row);
    lv_obj_set_size(button_l,
                    BUTTON_SIZE,
                    BUTTON_SIZE);
    lv_obj_add_event_cb(button_l, on_left, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_l = lv_label_create(button_l);
    lv_label_set_text(label_l, LV_SYMBOL_LEFT);
    lv_obj_center(label_l);

    // add the middle pager
    lv_obj_t *cont = lv_obj_create(row);
    lv_obj_set_size(cont,
                    LV_PCT(100) - BUTTON_SIZE * 2 - 10,
                    LV_PCT(100) - BUTTON_SIZE * 2 - 10);
    lv_obj_set_style_pad_all(cont, 0, 0);
    // lv_obj_set_style_pad_row(cont, 0, 0);
    // lv_obj_set_style_pad_column(cont, 0, 0);
    lv_obj_set_scroll_dir(cont, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    {
        // lv_obj_t *page = lv_obj_create(cont);
        // lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
        // lv_obj_set_style_pad_all(page, 0, 0);
        // lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

        // lv_obj_t *llll = lv_label_create(cont);
        // lv_label_set_text(llll, "a");
        // lv_obj_set_style_text_font(llll, &lv_font_montserrat_10, 0);

        // lv_obj_center(label);
    }

    lv_obj_t *button_r = lv_btn_create(row);
    lv_obj_set_size(button_r,
                    BUTTON_SIZE,
                    BUTTON_SIZE);

    lv_obj_add_event_cb(button_r, on_left, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_r = lv_label_create(button_r);
    lv_label_set_text(label_r, LV_SYMBOL_RIGHT);
    lv_obj_center(label_r);

    // pager_goto(0, LV_ANIM_OFF);
}