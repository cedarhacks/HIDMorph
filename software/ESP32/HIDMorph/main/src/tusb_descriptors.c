#include "tusb.h"

// -------------------- Device Descriptor --------------------
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    .bDeviceClass = 0x00, // each interface specifies its own class
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0xCafe,  // change me
    .idProduct = 0x4011, // change me
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

// -------------------- HID Report Descriptor --------------------
// Simple boot keyboard; swap for your custom report if desired.
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    return desc_hid_report;
}

// -------------------- Configuration Descriptor --------------------
enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define EPNUM_MSC_OUT 0x01
#define EPNUM_MSC_IN 0x81
#define EPNUM_HID_IN 0x82

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN + TUD_HID_DESC_LEN)

uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power (mA)
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // MSC: interface, string index, EP OUT & IN, EP size
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 4, EPNUM_MSC_OUT, EPNUM_MSC_IN, 64),

    // HID: interface, string index, boot protocol, report desc len, EP IN, size, interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 5, HID_ITF_PROTOCOL_KEYBOARD,
                       sizeof(desc_hid_report), EPNUM_HID_IN, CFG_TUD_HID_EP_BUFSIZE, 10)};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

// -------------------- Strings --------------------
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: English (0x0409)
    "CedarHacks",               // 1: Manufacturer
    "Composite MSC+HID",        // 2: Product
    "123456",                   // 3: Serial
    "Mass Storage",             // 4: MSC
    "Keyboard HID",             // 5: HID
};

static uint16_t _desc_str[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    if (index == 0) {
        _desc_str[1] = (uint16_t)(string_desc_arr[0][1] << 8) | string_desc_arr[0][0];
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 + 1);
        return _desc_str;
    }

    const char *str = string_desc_arr[index];
    uint8_t len = (uint8_t)strlen(str);
    if (len > 31) len = 31;

    for (uint8_t i = 0; i < len; i++) _desc_str[1 + i] = str[i];
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);

    return _desc_str;
}
