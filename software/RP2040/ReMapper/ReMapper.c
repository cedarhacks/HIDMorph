
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"

#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "bsp/board_api.h"
#include "tusb.h"

#include "hid_messages.h"

const int DEBUG_LED_PIN = 5;
static bool led_toggle = false;

spi_inst_t *spi_controller;
char out_buffer[255];
char in_buffer[255];

void init_spi() {

    int SPI_RX_PIN = 16;
    int SPI_TX_PIN = 19;
    int SPI_SCK_PIN = 18;
    int SPI_CS_PIN = 17;

    spi_controller = spi0_hw;

    // Enable SPI 0 at 50 MHz and connect to GPIOs
    spi_init(spi_controller, 50*100 * 1000);
    spi_set_slave(spi_controller, true);

    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CS_PIN, GPIO_FUNC_SPI);
    spi_set_format(spi_controller, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_LSB_FIRST);
}


// Invoked when device with HID interface is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
    printf("HID device address %d mounted\n", dev_addr);
    tuh_hid_receive_report(dev_addr, instance); // Start receiving reports

    HID_MESSAGE_PACKET_t hid_message;
    strncpy(hid_message.sync_word, "DREAM", 5);
    hid_message.type = DEVICE_CONNECTED;
    hid_message.message.connected.dev_addr = dev_addr;
    memcpy(out_buffer, &hid_message, sizeof(hid_message));

    int bytes = spi_write_blocking(spi_controller, out_buffer, sizeof(hid_message));
    printf("WROTE CONNECT TO SPI: %dbytes\n", bytes);

}

// Invoked when device with HID is unmounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID device address %d unmounted\n", dev_addr);

    HID_MESSAGE_PACKET_t hid_message;
    strncpy(hid_message.sync_word, "DREAM", 5);
    hid_message.type = DEVICE_DISCONNECTED;
    hid_message.message.disconnected.dev_addr = dev_addr;
    memcpy(out_buffer, &hid_message, sizeof(hid_message));

    int bytes = spi_write_blocking(spi_controller, out_buffer, sizeof(hid_message));
    printf("WROTE DISCONNECT TO SPI: %dbytes\n", bytes);

}

// Called when a HID report is received
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const *report, uint16_t len) {

    // printf("HID report from device %d, instance %d: ", dev_addr, instance);
    // for (int i = 0; i < len; i++) {
    //     printf("%02X ", report[i]);
    // }
    // printf("\n");

    // Continue receiving next report
    tuh_hid_receive_report(dev_addr, instance);

    HID_MESSAGE_PACKET_t hid_message;
    strncpy(hid_message.sync_word, "DREAM", 5);
    hid_message.type = NEW_REPORT;
    hid_message.message.new_report.dev_addr = dev_addr;
    hid_message.message.new_report.length = len;
    hid_message.message.new_report.report_id = 0;
    memcpy(hid_message.message.new_report.data, report, len > 64 ? 64 : len);

    memcpy(out_buffer, &hid_message, sizeof(hid_message));
    int bytes = spi_write_blocking(spi_controller, out_buffer, sizeof(hid_message));
    // printf("WROTE REPORT TO SPI: %dbytes\n", bytes);

}

void init_hid() {
    tusb_init();
}

void step_hid() {
    tuh_task(); // tinyusb device task
}

int main() {
    stdio_init_all();

    printf("\n\n Started ReMapper V1\n");

    // debug gpio
    gpio_init(DEBUG_LED_PIN);
    gpio_set_dir(DEBUG_LED_PIN, GPIO_OUT);

    // board
    board_init();
    // spi
    init_spi();
    // USB
    init_hid();

    while (true) {
        gpio_put(DEBUG_LED_PIN, led_toggle);
        led_toggle = !led_toggle;

        step_hid();
        sleep_us(1);
    }
}
