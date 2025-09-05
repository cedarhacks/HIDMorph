#include "W25Q128J.h"

const char *TAG = "W25Q128J";

bool init_W25Q128J(W25Q128J_t *device, int SPI_DEV) {

    // init the spi bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_DEV, &device->bus, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "Initialized spi bus on dev: %d  ... now attaching flash\n", SPI_DEV);

    esp_flash_spi_device_config_t new_cfg = {
        .host_id = SPI_DEV,
        .cs_id = 0, // CS index on this host
        .cs_io_num = device->PIN_CS,
        .io_mode = SPI_FLASH_DIO, // start with DIO; works on most boards
        .speed = ESP_FLASH_40MHZ, // you can try 80MHz later
        .input_delay_ns = 0,
        .freq_mhz = 40.0, // 20 MHz
    };

    device->dev_cfg = new_cfg;

    ESP_ERROR_CHECK(spi_bus_add_flash_device(&device->ext_flash, &device->dev_cfg));
    ESP_ERROR_CHECK(esp_flash_init(device->ext_flash));

    return true;
}

bool read_W25Q128J(W25Q128J_t *device, void *buff, uint32_t addr, uint32_t len) {
    esp_err_t err = esp_flash_read(device->ext_flash, buff, addr, len);
    if (err != ESP_OK) {
        return false;
    }

    return true;
}

bool write_W25Q128J(W25Q128J_t *device, void *buff, uint32_t addr, uint32_t len) {
    esp_err_t err = esp_flash_write(device->ext_flash, buff, addr, len);
    if (err != ESP_OK) {
        return false;
    }

    return true;
}

bool erase_all_W25Q128J(W25Q128J_t *device) {
    esp_err_t err = esp_flash_erase_chip(device->ext_flash);
    if (err != ESP_OK) {
        return false;
    }

    return true;
}

bool erase_region_W25Q128J(W25Q128J_t *device, uint32_t start, uint32_t len) {
    esp_err_t err = esp_flash_erase_region(device->ext_flash, start, len);
    if (err != ESP_OK) {
        return false;
    }

    return true;
}

uint32_t get_id_W25Q128J(W25Q128J_t *device){
    uint32_t out;
    esp_flash_read_id(device->ext_flash, &out);
    return out;
}

uint32_t get_size_W25Q128J(W25Q128J_t *device){
    uint32_t out;
    esp_flash_get_physical_size(device->ext_flash, &out);
    return out;
}


void print_info_W25Q128J(W25Q128J_t *device) {
}