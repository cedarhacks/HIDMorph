#include "task_webhost.h"
#include "webserver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void task_webhost(void *args) {
    webserver_init();

    while (1) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        webserver_step();
    }
}