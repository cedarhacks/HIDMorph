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

/*
 * W25Q128JV Flash Memory Overview:
 *
 * - Organized into 65,536 pages (256 bytes each)
 * - Up to 256 bytes can be programmed at a time (per page)
 *
 * Erase Options:
 *   - 4KB sectors (groups of 16 pages)
 *   - 32KB blocks (groups of 128 pages)
 *   - 64KB blocks (groups of 256 pages)
 *   - Entire chip erase
 *
 * - Total: 4,096 erasable sectors and 256 erasable blocks
 * - Small 4KB sectors allow flexible data/parameter storage
 */


typedef struct W25Q128J{
    spi_bus_config_t bus;
    esp_flash_spi_device_config_t dev_cfg;
    int PIN_CS;
    esp_flash_t *ext_flash;
} W25Q128J_t;


bool init_W25Q128J( W25Q128J_t* device, int SPI_DEV );
bool read_W25Q128J(W25Q128J_t *device, void *buff, uint32_t addr, uint32_t len);
bool write_W25Q128J(W25Q128J_t *device, void *buff, uint32_t addr, uint32_t len);
bool erase_all_W25Q128J(W25Q128J_t *device);
bool erase_region_W25Q128J(W25Q128J_t *device, uint32_t start, uint32_t len);

uint32_t get_id_W25Q128J(W25Q128J_t *device);
uint32_t get_size_W25Q128J(W25Q128J_t *device);

void print_info_W25Q128J(W25Q128J_t *device); // todo