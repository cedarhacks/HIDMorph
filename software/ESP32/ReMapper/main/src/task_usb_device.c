#include "task_usb_device.h"

#include "driver/gpio.h"
#include "tinyusb.h"
#include "tusb.h"
#include "tusb_msc_storage.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"

#include "extflash_fs.h"
#include "W25Q128J.h"

static const char *TAG = "USB OUT";

extern W25Q128J_t memory_chip;
extern ExtFlashFs_t ext_flash_fs;

// ------------------------------ HID CALLBACKS ---------------------
const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD))};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
}

// ------------------------------ DONE HID CALLBACKS ----------------

// ------------------------------ MSC CALLBACKS ---------------------

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

static tusb_desc_device_t msc_descriptor_config = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A, // This is Espressif VID. This needs to be changed according to Users / Customers
    .idProduct = 0x4002,
    .bcdDevice = 0x100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01};

static char const *msc_string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "TinyUSB",                  // 1: Manufacturer
    "TinyUSB Device",           // 2: Product
    "123456",                   // 3: Serials
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

    ESP_LOGI(TAG, "START BROTHER: WL handler: %d\n", ext_flash_fs.wl);

    // mount the fs

    tinyusb_config_t cfg = {0};
    cfg.device_descriptor = &msc_descriptor_config; // or NULL to use IDF defaults
    cfg.string_descriptor = msc_string_desc_arr;
    cfg.configuration_descriptor = msc_fs_configuration_desc;

    const tinyusb_msc_spiflash_config_t cfg_spi = {
        .wl_handle = ext_flash_fs.wl,
        .callback_mount_changed = storage_mount_changed_cb, /* First way to register the callback. This is while initializing the storage. */
        .mount_config.max_files = 5,
    };

    ESP_ERROR_CHECK(tinyusb_msc_storage_init_spiflash(&cfg_spi));
    ESP_ERROR_CHECK(tinyusb_driver_install(&cfg));

    ESP_LOGI(TAG, "DONE BROTHER");

    return true;
}

// we either want to init the USB device in MSC or HID mode
// but we need to do this once before installing the drivers
void task_usb_device(void *argv) {

    bool programming_mode = true;

    if (programming_mode) {
        usb_init_msc();
    } else {
        // usb_init_hid();
    }

    while (1) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}