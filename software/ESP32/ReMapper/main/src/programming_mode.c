#include "programming_mode.h"
#include "W25Q128J.h"

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

void run_programming_mode() {
    // we need to initalize our qspi

    memory_chip.bus.mosi_io_num = PIN_NUM_MOSI;
    memory_chip.bus.miso_io_num = PIN_NUM_MISO;
    memory_chip.bus.sclk_io_num = PIN_NUM_CLK;
    memory_chip.bus.quadwp_io_num = PIN_NUM_IO2;
    memory_chip.bus.quadhd_io_num = PIN_NUM_IO3;
    memory_chip.bus.max_transfer_sz = 4096;
    memory_chip.PIN_CS = PIN_NUM_CS;

    init_W25Q128J(&memory_chip, SPI3_HOST);

    printf("\n Flash Size: 0x%0lx\n", get_size_W25Q128J(&memory_chip));
    printf("Flash ID: 0x%0lx\n\n", get_id_W25Q128J(&memory_chip));

    uint8_t read_buff[255];

    uint8_t write_buff[255];
    for (int i = 0; i < 255; i++)
        write_buff[i] = i % 50;

    read_W25Q128J(&memory_chip, read_buff, 0x0, 255);
    pprint_buff(read_buff, 255);

    printf("\n Writing now\n");
    write_W25Q128J(&memory_chip, write_buff, 0x0, 255);
    read_W25Q128J(&memory_chip, read_buff, 0x0, 255);
    pprint_buff(read_buff, 255);

    printf("\n erase region \n");
    erase_region_W25Q128J(&memory_chip, 0x0, 4096); // can only erase in sector sizes
    read_W25Q128J(&memory_chip, read_buff, 0x0, 255);
    pprint_buff(read_buff, 255);


    // printf("\n erase all \n");
    // erase_all_W25Q128J(&memory_chip);
    // read_W25Q128J(&memory_chip, read_buff, 0x0, 255);
    // pprint_buff(read_buff, 255);


    printf("\n Done\n");
}