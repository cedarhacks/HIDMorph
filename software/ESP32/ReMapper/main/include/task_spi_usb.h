#pragma once

#include "hid_messages.h"
#include "hid_event.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

void task_spi_usb(void *args);
