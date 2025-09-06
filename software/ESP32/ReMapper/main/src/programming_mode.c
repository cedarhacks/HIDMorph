#include "programming_mode.h"
#include "extflash_fs.h"
#include "W25Q128J.h"
#include "esp_log.h"

#include "tusb_msc_storage.h" // ESP-IDF helper for MSC
#include "tusb.h"
#include "tusb_config.h"
#include "class/msc/msc_device.h"
#include "tusb_msc_storage.h"

#define SECTOR_SIZE 512
#define BOARD_TUD_RHPORT 0

static const char *TAG = "PROGRAMMING MODE";

W25Q128J_t memory_chip;
ExtFlashFs_t ext_flash_fs;

// ---- TinyUSB MSC Callbacks ----
// return number of sectors
void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
    (void)lun;
    size_t size = wl_size(ext_flash_fs.wl); // total bytes
    size_t sec_size = SECTOR_SIZE;
    *block_count = size / sec_size;
    *block_size = sec_size;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                          void *buffer, uint32_t bufsize) {

    esp_err_t err = wl_read(ext_flash_fs.wl,
                            lba * SECTOR_SIZE + offset,
                            buffer,
                            bufsize);
    return (err == ESP_OK);
}
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                           uint8_t *buffer, uint32_t bufsize) {

    esp_err_t err = wl_write(ext_flash_fs.wl,
                             lba * SECTOR_SIZE + offset,
                             buffer,
                             bufsize);
    return (err == ESP_OK);
}

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8],
                        uint8_t product_id[16], uint8_t product_rev[4]) {

    const char vid[] = "CEDARHCK";
    const char pid[] = "EXTFLASH";
    const char rev[] = "1.0";
    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize) {
    int32_t ret;

    ESP_LOGD(TAG, "tud_msc_scsi_cb() invoked. bufsize=%d", bufsize);

    switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        /* SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL is the Prevent/Allow Medium Removal
        command (1Eh) that requests the library to enable or disable user access to
        the storage media/partition. */
        ESP_LOGI(TAG, "tud_msc_scsi_cb() invoked: SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL");
        ret = 0;
        break;
    default:
        ESP_LOGW(TAG, "tud_msc_scsi_cb() invoked: %d", scsi_cmd[0]);
        tud_msc_set_sense(lun,
                          0x05,
                          0x20,
                          0x00);
        ret = -1;
        break;
    }
    return ret;
}

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
    stat = extfs_setup(&ext_flash_fs, memory_chip.ext_flash, "/ext", "myextfs");
    ESP_LOGI(TAG, "Mount External flash as vfs wl: %d", stat == true);

    extfs_unmount(&ext_flash_fs, "/ext");
    tud_init(BOARD_TUD_RHPORT); // start MSC

    while (1) {
        tud_task(); // TinyUSB background task
        vTaskDelay(1);
    }

    tud_deinit(BOARD_TUD_RHPORT); // stop MSC

    extfs_mount_fatfs(&ext_flash_fs, "/ext", "myextfs");
}
