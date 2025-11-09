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
#include "task_display.h"
#include "task_lua_vm.h"
#include "task_input_manager.h"
#include "programming_mode.h"
#include "hid_q.h"

#include "esp_log.h"

int flash_duration = 500;

void app_main(void) {

    int programming_mode_gpio = GPIO_NUM_1;

    // Programming Mode switch  -> pullup
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << programming_mode_gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    volatile bool is_programming_mode = gpio_get_level(programming_mode_gpio) == 0;

    // initialize the hid queue
    hid_queue_init();

    // display task
    xTaskCreate(task_display,
                "display",
                8192,
                NULL,
                1,
                NULL);

    // input_manager_task
    xTaskCreate(task_input_manager,
                "input_manager",
                2048,
                NULL,
                1,
                NULL);

    vTaskDelay(50 / portTICK_PERIOD_MS);

    if (is_programming_mode) {
        flash_duration = 100;

        ESP_LOGI("main", "Entering Programming Mode");

        // sets up the file system in the qspi
        run_programming_mode();

        // in progamming mode we run the MSC task to allow the user to edit programs
        xTaskCreate(task_usb_storage_device,
                    "usb_storage",
                    4096,
                    NULL,
                    10,
                    NULL);

    } else {
        flash_duration = 1000;

        // check the boot mode
        xTaskCreate(task_spi_usb,
                    "spi_usb",
                    4096,
                    NULL,
                    10,
                    NULL);

        xTaskCreate(task_lua_vm,
                    "lua_vm",
                    8192,
                    NULL,
                    10,
                    NULL);

        xTaskCreate(task_gamepad_out,
                    "gamepad",
                    4096,
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
