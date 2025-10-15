#pragma once
#include "lvgl.h"

typedef struct {
    lv_obj_t *row;
    lv_obj_t *button_l;
    lv_obj_t *button_r;
    lv_obj_t *page;
    int current_i;
    int num_items;
    char **items;
} list_selector_t;

void list_selector_init(lv_obj_t *parent, list_selector_t *selector);
void list_selector_scroll_to(list_selector_t *selector, int index);