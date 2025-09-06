#include "programming_mode.h"
#include "extflash_fs.h"
#include "W25Q128J.h"
#include "esp_log.h"

static const char *TAG = "PROGRAMMING MODE";

W25Q128J_t memory_chip;

int PIN_NUM_MOSI = 12;
int PIN_NUM_MISO = 13;
int PIN_NUM_CLK = 11;
int PIN_NUM_IO2 = 14;
int PIN_NUM_IO3 = 21;
int PIN_NUM_CS = 17;

void pprint_buff(uint8_t *buff, int len) {
    for (int i = 0; i < len; i++) {
        if (i % 16 == 0 && i != 0) {
            printf("\n");
        }
        printf("%x ", buff[i]);
    }
}

void fatfs_test(void) {
    // 1. Write a test file
    FILE *f = fopen("/ext/test.txt", "w"); // "/ext" = your mount point
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello FATFS! Count=%d\n", 123);
    fclose(f);
    ESP_LOGI(TAG, "Wrote test.txt");

    // 2. Read it back
    f = fopen("/ext/test.txt", "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char buf[64];
    if (fgets(buf, sizeof(buf), f)) {
        ESP_LOGI(TAG, "Read back: %s", buf);
    } else {
        ESP_LOGE(TAG, "Read failed");
    }
    fclose(f);

    // 3. Append something
    f = fopen("/ext/test.txt", "a");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for appending");
        return;
    }
    fprintf(f, "Appended line!\n");
    fclose(f);
    ESP_LOGI(TAG, "Appended to test.txt");
}

void run_programming_mode() {
    bool stat;

    // initialize the qspi CHIP here
    memory_chip.bus.mosi_io_num = PIN_NUM_MOSI;
    memory_chip.bus.miso_io_num = PIN_NUM_MISO;
    memory_chip.bus.sclk_io_num = PIN_NUM_CLK;
    memory_chip.bus.quadwp_io_num = PIN_NUM_IO2;
    memory_chip.bus.quadhd_io_num = PIN_NUM_IO3;
    memory_chip.bus.max_transfer_sz = 4096;
    memory_chip.PIN_CS = PIN_NUM_CS;
    stat = init_W25Q128J(&memory_chip, SPI3_HOST);

    ESP_LOGI(TAG, "Flash init: %d", stat == true);
    ESP_LOGI(TAG, "Flash Size: 0x%0lx", get_size_W25Q128J(&memory_chip));
    ESP_LOGI(TAG, "Flash ID: 0x%0lx", get_id_W25Q128J(&memory_chip));

    // now the goal is to mount an external parition to this qspi flash
    ExtFlashFs_t ext_part;
    stat = extfs_setup(&ext_part, memory_chip.ext_flash, "/ext", "myextfs");
    ESP_LOGI(TAG, "Mount External flash as vfs wl: %d", stat == true);

    

}