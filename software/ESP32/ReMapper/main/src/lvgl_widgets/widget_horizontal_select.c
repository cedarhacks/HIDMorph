#include "widget_horizontal_select.h"

void list_selector_scroll_to(list_selector_t *selector, int index) {
    lv_obj_scroll_to_view(lv_obj_get_child(selector->page, index), LV_ANIM_OFF);
}

void list_selector_init(lv_obj_t *parent, list_selector_t *selector) {

    lv_coord_t parent_width = lv_obj_get_width(parent);
    lv_coord_t parent_height = lv_obj_get_height(parent);

    int button_size = 20;

    lv_obj_t *row = lv_obj_create(parent);
    selector->row = row;

    lv_obj_set_size(row, LV_PCT(100), button_size); // row height = button size
    lv_obj_set_style_pad_all(row, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_center(row);

    // Left arrow
    lv_obj_t *button_l = lv_btn_create(row);
    selector->button_l = button_l;

    lv_obj_set_size(button_l, button_size, button_size);
    // lv_obj_add_event_cb(button_l, on_left, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_l = lv_label_create(button_l);
    lv_label_set_text(label_l, LV_SYMBOL_LEFT);
    lv_obj_center(label_l);

    // Middle pager container (fills remaining width, fixed height)
    lv_obj_t *cont = lv_obj_create(row);
    lv_obj_remove_style_all(cont);
    lv_obj_set_width(cont, LV_PCT(100));
    lv_obj_set_height(cont, button_size); // <-- key: give it a real height
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_grow(cont, 1);

    // Horizontal scrolling page
    lv_obj_t *page = lv_obj_create(cont);
    selector->page = page;
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100)); // now % works (parent has a height)
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // scrolling + snapping
    lv_obj_add_flag(page, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_set_scroll_dir(page, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(page, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_anim_time(page, 80, 0);

    // Full-width snappable items
    for (int i = 0; i < selector->num_items; i++) {
        lv_obj_t *item = lv_obj_create(page);
        lv_obj_remove_style_all(item);
        lv_obj_set_size(item, LV_PCT(100), LV_PCT(100)); // one-viewport page
        lv_obj_add_flag(item, LV_OBJ_FLAG_SNAPPABLE);

        // center label inside
        lv_obj_set_layout(item, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(item, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *label_num = lv_label_create(item);
        lv_label_set_text_fmt(label_num, "%d", i);
        lv_obj_set_style_text_font(label_num, &lv_font_montserrat_10, 0);
        lv_label_set_long_mode(label_num, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_width(label_num, LV_PCT(25));

        lv_obj_t *label = lv_label_create(item);
        lv_label_set_text_fmt(label, "%s", selector->items[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_10, 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_width(label, LV_PCT(75));
    }

    // Right arrow
    lv_obj_t *button_r = lv_btn_create(row);
    selector->button_r = button_r;

    lv_obj_set_size(button_r, button_size, button_size);
    // lv_obj_add_event_cb(button_r, on_right, LV_EVENT_CLICKED, NULL); // fixed
    lv_obj_t *label_r = lv_label_create(button_r);
    lv_label_set_text(label_r, LV_SYMBOL_RIGHT);
    lv_obj_center(label_r);
}