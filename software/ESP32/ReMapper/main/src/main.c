#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "task_spi_usb.h"
#include "task_webhost.h"
#include "task_gamepad_out.h"
#include "task_usb_storage_device.h"
#include "programming_mode.h"

#include "esp_log.h"

int flash_duration = 500;

void app_main(void) {

    volatile bool is_programming_mode = false;

    if (is_programming_mode) {
        flash_duration = 100;

        ESP_LOGI("main", "Entering Programming Mode");

        // sets up the file system in the qspi
        run_programming_mode();

        // in progamming mode we run the MSC task to allow the user to edit programs
        xTaskCreate(task_usb_storage_device,
                    "usb_storage",
                    20480,
                    NULL,
                    10,
                    NULL);

    } else {
        flash_duration = 1000;

        // check the boot mode
        xTaskCreate(task_spi_usb,
                    "spi_usb",
                    20480,
                    NULL,
                    10,
                    NULL);

        // no web task 
        // xTaskCreate(task_webhost,
        //             "webhost",
        //             20480,
        //             NULL,
        //             1,
        //             NULL);

        xTaskCreate(task_gamepad_out,
                    "gamepad",
                    20480,
                    NULL,
                    10,
                    NULL);
    }

    gpio_reset_pin(8);
    gpio_set_direction(8, GPIO_MODE_OUTPUT);
    int blink_val = 0;

    while (1) {
        vTaskDelay(flash_duration / portTICK_PERIOD_MS);
        gpio_set_level(8, blink_val);
        blink_val = !blink_val;
    }
}
