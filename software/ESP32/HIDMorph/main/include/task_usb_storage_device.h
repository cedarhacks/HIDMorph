#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"


// this task will take up the USB lines an register a Composite USB device
void task_usb_storage_device( void* argv );