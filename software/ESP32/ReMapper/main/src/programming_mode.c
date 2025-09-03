#include "programming_mode.h"
#include "W25Q128J.h"

W25Q128J_t memory_chip;

int PIN_NUM_MOSI = 12;
int PIN_NUM_MISO = 13;
int PIN_NUM_CLK = 11;
int PIN_NUM_IO2 = 14;
int PIN_NUM_IO3 = 21;
int PIN_NUM_CS = 17;

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
}