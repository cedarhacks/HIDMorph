#include "task_usb_device.h"
#include "tusb_config.h"

static const char* TAG = "USB OUT";

void task_usb_device( void* argv ){


    while(1){
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}