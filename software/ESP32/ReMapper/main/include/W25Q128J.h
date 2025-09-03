#pragma once
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_err.h"


typedef struct W25Q128J{
    spi_bus_config_t bus;
    esp_flash_spi_device_config_t dev_cfg;
    int PIN_CS;
    esp_flash_t *ext_flash;
} W25Q128J_t;


bool init_W25Q128J( W25Q128J_t* device, int SPI_DEV );

