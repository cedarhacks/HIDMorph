#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "task_spi_usb.h"
#include "task_webhost.h"
#include "task_gamepad_out.h"

void app_main(void) {

    // check the boot mode
    xTaskCreate(task_spi_usb,
                "spi_usb",
                20480,
                NULL,
                10,
                NULL);

    xTaskCreate(task_webhost,
                "webhost",
                20480,
                NULL,
                1,
                NULL);

    xTaskCreate(task_gamepad_out,
                "gamepad",
                20480,
                NULL,
                10,
                NULL);

    gpio_reset_pin(8);
    gpio_set_direction(8, GPIO_MODE_OUTPUT);
    int blink_val = 0;

    while (1) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(8, blink_val);
        blink_val = !blink_val;
    }
}
