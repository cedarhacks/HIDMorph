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
    };

    device->dev_cfg = new_cfg;

    // CRASHING!!!!
    // device->ext_flash = NULL;
    // ESP_ERROR_CHECK(spi_bus_add_flash_device(&device->ext_flash, &device->dev_cfg));
    // ESP_ERROR_CHECK(esp_flash_init(device->ext_flash));

    return true;
}
