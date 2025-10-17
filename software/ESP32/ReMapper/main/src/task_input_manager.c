#include "task_input_manager.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static Button_t button_next;
static Button_t button_prev;
static Button_t button_select;

static Button_t *input_events[MAX_INPUTS];
static int num_input_events = 0;

CLICKED_CB clicked_callbacks[MAX_CLICKED_CALLBACKS];
int clicked_callbacks_i = 0;

static const char *TAG = "INPUT MANAGER";

void task_input_manager(void *args) {

    input_events[0] = &button_next;
    input_events[1] = &button_prev;
    input_events[2] = &button_select;
    num_input_events = 3;

    init_debounce_button(&button_next, BUTTON_NEXT);
    init_debounce_button(&button_prev, BUTTON_PREV);
    init_debounce_button(&button_select, BUTTON_SELECT);

    while (1) {
        vTaskDelay(5 / portTICK_PERIOD_MS);

        for (int i = 0; i < num_input_events; i++) {
            step_debounce_button(input_events[i]);

            if (is_clicked_debounce_button(input_events[i])) {
                ESP_LOGI(TAG, "Button Clicked: %d", input_events[i]->GPIO_PIN);
            
                for( int ci = 0; ci < clicked_callbacks_i; ci++){
                    clicked_callbacks[ci](input_events[i]->GPIO_PIN);
                }
            }
        }
    }
}

void input_listen_click(CLICKED_CB cb){
    if( clicked_callbacks_i < MAX_CLICKED_CALLBACKS-1){
        clicked_callbacks[clicked_callbacks_i] = cb;
        clicked_callbacks_i += 1;
    }
}