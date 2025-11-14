#include "task_spi_usb.h"
#include "hid_parsing.h"

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hid_q.h"

// Prepare data to send
uint8_t tx_data[1024] = {0};
uint8_t rx_data[1024] = {0};

#define NEW_EVENTS_SIZE (30)
Event_t new_events[NEW_EVENTS_SIZE];

void print_event(Event_t *event) {
    printf("EVENT: %s %c\n", event->type == EVENT_KEY_RELEASED ? "RELEASED" : "PRESSED", event->ascii);
}

void task_spi_usb(void *args) {

    spi_device_handle_t rpi_spi;

    const int PIN_NUM_MOSI = 4;
    const int PIN_NUM_MISO = 5;
    const int PIN_NUM_CLK = 6;
    const int PIN_NUM_CS = 7;

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1, // Not used
        .quadhd_io_num = -1, // Not used
        .max_transfer_sz = 32,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST,
                                       &buscfg,
                                       SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000, // Clock out at 1 MHz
        .mode = 0,                          // SPI mode 0
        .spics_io_num = PIN_NUM_CS,         // CS pin
        .queue_size = 1,                    // We want to be able to queue 7 transactions at a time
    };

    ret = spi_bus_add_device(SPI2_HOST,
                             &devcfg,
                             &rpi_spi);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("SPI", "DONE WITH SPI INITIALIZATION: spi ptr: %p\n", rpi_spi);

    spi_transaction_t t = {
        .length = 8 * sizeof(HID_MESSAGE_PACKET_t),
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    while (1) {

        memset(rx_data, 0, sizeof(rx_data));

        ret = spi_device_transmit(rpi_spi, &t);
        ESP_ERROR_CHECK(ret);

        HID_MESSAGE_PACKET_t hid_packet;
        memcpy(&hid_packet, rx_data, sizeof(HID_MESSAGE_PACKET_t));

        if (hid_packet.sync_word[0] == 'D' && hid_packet.sync_word[1] == 'R' && hid_packet.sync_word[2] == 'E' && hid_packet.sync_word[3] == 'A' && hid_packet.sync_word[4] == 'M') {

            int num_events = parse_hid_packet(&hid_packet, new_events, NEW_EVENTS_SIZE);

            for (int i = 0; i < num_events; i++) {

                // key pressed
                if (new_events[i].type == EVENT_KEY_PRESSED) {
                    // ESP_LOGI("[usb]", "DOWN: %c", new_events[i].ascii);
                    uint8_t temp_keycodes[6];
                    temp_keycodes[0] = new_events[i].keycode;
                    hid_post_keyboard(&input_events_q, 0, temp_keycodes, false,  0); // not released
                }

                // key release
                if (new_events[i].type == EVENT_KEY_RELEASED) {
                    // ESP_LOGI("[usb]", "UP: %c", new_events[i].ascii);
                    uint8_t temp_keycodes[6];
                    temp_keycodes[0] = new_events[i].keycode;
                    hid_post_keyboard(&input_events_q, 0, temp_keycodes, true,  0); // not released
                }
            }
            // ESP_LOGI("[usb]", "---------");
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
