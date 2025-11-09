#include "task_hid_out.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void task_hid_out(void *args) {
    gamepad_hid_init();

    while (1) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        gamepad_hid_step();
    }
}
