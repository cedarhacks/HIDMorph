#include "task_usb_storage_device.h"

#include "driver/gpio.h"
#include "tinyusb.h"
#include "tusb.h"
#include "tusb_msc_storage.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"
#include "extflash_fs.h"
#include "W25Q128J.h"

static const char *TAG = "USB OUT";
static W25Q128J_t memory_chip;
static ExtFlashFs_t ext_flash_fs;


// callback that is delivered when storage is mounted/unmounted by application.
static void storage_mount_changed_cb(tinyusb_msc_event_t *event) {
    ESP_LOGI(TAG, "Storage mounted to application: %s", event->mount_changed_data.is_mounted ? "Yes" : "No");
}

#define EPNUM_MSC 1
#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_TOTAL
};

enum {
    EDPT_CTRL_OUT = 0x00,
    EDPT_CTRL_IN = 0x80,
    EDPT_MSC_OUT = 0x01,
    EDPT_MSC_IN = 0x81,
};

static tusb_desc_device_t msc_descriptor_config = {.bLength = sizeof(tusb_desc_device_t),
                                                   .bDescriptorType = TUSB_DESC_DEVICE,
                                                   .bcdUSB = 0x0200,
                                                   .bDeviceClass = TUSB_CLASS_MISC,
                                                   .bDeviceSubClass = MISC_SUBCLASS_COMMON,
                                                   .bDeviceProtocol = MISC_PROTOCOL_IAD,
                                                   .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
                                                   .idVendor = 0x303A,
                                                   .idProduct = 0x4002,
                                                   .bcdDevice = 0x100,
                                                   .iManufacturer = 0x01,
                                                   .iProduct = 0x02,
                                                   .iSerialNumber = 0x03,
                                                   .bNumConfigurations = 0x01};

static char const *msc_string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "CedarHacks",               // 1: Manufacturer
    "CedarHacks HIDMorph",      // 2: Product
    "111111",                   // 3: Serials
    "Example MSC",              // 4. MSC
};

static uint8_t const msc_fs_configuration_desc[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EDPT_MSC_OUT, EDPT_MSC_IN, 64),
};

bool usb_init_msc() {
    // Mount FAT on wear-levelled SPI flash; capture wl_handle_t.
    const esp_vfs_fat_mount_config_t mount_cfg = {.max_files = 8,
                                                  .format_if_mount_failed = true,
                                                  .allocation_unit_size = 4096};

    tinyusb_config_t cfg = {0};
    cfg.device_descriptor = &msc_descriptor_config; // or NULL to use IDF defaults
    cfg.string_descriptor = msc_string_desc_arr;
    cfg.configuration_descriptor = msc_fs_configuration_desc;

    const tinyusb_msc_spiflash_config_t cfg_spi = {
        .wl_handle = ext_flash_fs.wl,
        .callback_mount_changed = storage_mount_changed_cb,
        .mount_config.max_files = 5,
    };

    ESP_ERROR_CHECK(tinyusb_msc_storage_init_spiflash(&cfg_spi));
    ESP_ERROR_CHECK(tinyusb_driver_install(&cfg));
    ESP_LOGI(TAG, "Done Instaling TinyUSB MSC Drivers");

    return true;
}

void task_usb_storage_device(void *argv) {

    usb_init_msc();

    while (1) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}