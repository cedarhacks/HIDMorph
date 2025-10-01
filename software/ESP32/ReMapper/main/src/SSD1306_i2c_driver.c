#include "SSD1306_i2c_driver.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "SSD1306";

void pack_pixels_to_ssd1306(uint8_t pixels[SSD1306_WIDTH * SSD1306_HEIGHT],
                            uint8_t buffer[SSD1306_WIDTH * SSD1306_PAGES]) {
    memset(buffer, 0, SSD1306_WIDTH * SSD1306_PAGES);

    for (int page = 0; page < SSD1306_PAGES; page++) {
        for (int x = 0; x < SSD1306_WIDTH; x++) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                int y = page * 8 + bit;
                if (y >= SSD1306_HEIGHT) break; // Skip if outside display

                int index = y * SSD1306_WIDTH + x;
                if (pixels[index]) {
                    byte |= (1 << bit); // LSB = top pixel of this page
                }
            }
            buffer[page * SSD1306_WIDTH + x] = byte;
        }
    }
}
void SSD1306_init(SSD1306_t *inst) {

    i2c_master_bus_config_t bus_config = {
        .i2c_port = 0,
        .sda_io_num = inst->i2c_data_pin,
        .scl_io_num = inst->i2c_clock_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &inst->bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = inst->i2c_address,
        .scl_speed_hz = inst->i2c_freq_hz,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(inst->bus_handle, &dev_config, &inst->dev_handle));

    esp_err_t stat = i2c_master_probe(inst->bus_handle, inst->i2c_address, 10);
    if (stat == ESP_OK) {
        printf("\nPROBE SUCCESS\n");
    } else {
        printf("\nPROBE FAIL\n");
    }
}

void SSD1306_setup_display(SSD1306_t *inst) {

    // boot sequence commands
    // Init sequence
    uint8_t init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
                       SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
                       0x80,                       // the suggested ratio 0x80
                       SSD1306_SETMULTIPLEX};      // 0xA8
    SSD1306_command_list(inst, init1, sizeof(init1));
    SSD1306_command(inst, SSD1306_HEIGHT - 1);

    uint8_t init2[] = {SSD1306_SETDISPLAYOFFSET,   // 0xD3
                       0x0,                        // no offset
                       SSD1306_SETSTARTLINE | 0x0, // line #0
                       SSD1306_CHARGEPUMP};        // 0x8D

    SSD1306_command_list(inst, init2, sizeof(init2));

    SSD1306_command(inst, 0x14);

    uint8_t init3[] = {SSD1306_MEMORYMODE, // 0x20
                       0x00,               // 0x0 act like ks0108
                       SSD1306_SEGREMAP | 0x1,
                       SSD1306_COMSCANDEC};
    SSD1306_command_list(inst, init3, sizeof(init3));

    uint8_t comPins = 0x02;
    inst->contrast = 0x8F;

    if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32)) {
        comPins = 0x02;
        inst->contrast = 0x8F;
    } else {
        // Other screen varieties -- TBD
    }

    SSD1306_command(inst, SSD1306_SETCOMPINS);
    SSD1306_command(inst, comPins);
    SSD1306_command(inst, SSD1306_SETCONTRAST);
    SSD1306_command(inst, inst->contrast);

    SSD1306_command(inst, SSD1306_SETPRECHARGE); // 0xd9
    SSD1306_command(inst, 0xF1);
    uint8_t init5[] = {
        SSD1306_SETVCOMDETECT, // 0xDB
        0x40,
        SSD1306_DISPLAYALLON_RESUME, // 0xA4
        SSD1306_NORMALDISPLAY,       // 0xA6
        SSD1306_DEACTIVATE_SCROLL,
        SSD1306_DISPLAYON}; // Main screen turn on

    SSD1306_command_list(inst, init5, sizeof(init5));
}

void SSD1306_command(SSD1306_t *inst, uint8_t command) {

    uint8_t buff[2];
    buff[0] = 0x00; // indicate command
    buff[1] = command;

    i2c_master_transmit(inst->dev_handle,
                        buff,
                        2,
                        100 / portTICK_PERIOD_MS);
}

void SSD1306_command_list(SSD1306_t *inst, uint8_t *commands, int len_commands) {
    if (len_commands >= MAX_COMMANDS - 1) {
        ESP_LOGE(TAG, "error can't send %d bytes, max: %d\n", len_commands, MAX_COMMANDS);
        return;
    }

    uint8_t buff[MAX_COMMANDS];
    buff[0] = 0x00; // indicate command
    memcpy(buff + 1, commands, len_commands);

    i2c_master_transmit(inst->dev_handle,
                        buff,
                        len_commands + 1,
                        100 / portTICK_PERIOD_MS);
}

void SSD1306_display(SSD1306_t *inst, uint8_t pixels[SSD1306_HEIGHT * SSD1306_WIDTH]) {
    static uint8_t packed_buffer[SSD1306_WIDTH * SSD1306_PAGES];
    pack_pixels_to_ssd1306(pixels, packed_buffer); // pack the pixels into a buffer to send

    uint8_t dlist1[] = {
        SSD1306_PAGEADDR,
        0,                   // Page start address
        0xFF,                // Page end (not really, but works here)
        SSD1306_COLUMNADDR}; // Column start address

    SSD1306_command_list(inst, dlist1, sizeof(dlist1));

    SSD1306_command(inst, 0);                   // Column start
    SSD1306_command(inst, (SSD1306_WIDTH - 1)); // Column end address

    // vTaskDelay(1 / portTICK_PERIOD_MS);

    uint16_t count = SSD1306_WIDTH * SSD1306_PAGES;
    uint8_t out[MAX_COMMANDS];

    out[0] = 0x40;
    uint16_t bytesOut = 1;
    uint8_t *ptr = packed_buffer;

    while (count--) {
        if (bytesOut >= MAX_COMMANDS) {
            i2c_master_transmit(inst->dev_handle,
                                out,
                                bytesOut,
                                100 / portTICK_PERIOD_MS);

            out[0] = 0x40;
            bytesOut = 1;
        }
        out[bytesOut] = *ptr++;
        bytesOut++;
    }

    i2c_master_transmit(inst->dev_handle,
                        out,
                        bytesOut,
                        100 / portTICK_PERIOD_MS);
}

void SSD1306_dim(SSD1306_t *inst, bool dim) {
    SSD1306_command(inst, SSD1306_SETCONTRAST);
    SSD1306_command(inst, dim ? 0 : inst->contrast);
}
