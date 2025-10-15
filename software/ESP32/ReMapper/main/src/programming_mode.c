#include "programming_mode.h"
#include "extflash_fs.h"
#include "W25Q128J.h"
#include "esp_log.h"

#include "tusb_msc_storage.h" // ESP-IDF helper for MSC
#include "tusb.h"
#include "tusb_config.h"
#include "class/msc/msc_device.h"
#include "tusb_msc_storage.h"

#include "task_display.h"
#include "lvgl.h"

#define SECTOR_SIZE 4096
#define BOARD_TUD_RHPORT 0

static const char *TAG = "PROGRAMMING MODE";

static W25Q128J_t memory_chip;
static ExtFlashFs_t ext_flash_fs;

LV_IMG_DECLARE(gear);

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

lv_obj_t *screen = NULL;
static lv_obj_t *s_label;
static lv_obj_t *img;
static lv_anim_t gear_anim;

static void gui_build(void *arg) {
    (void)arg;

    // gear
    img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, &gear);
    lv_obj_set_pos(img,
                   DISPLAY_W - (gear.header.w / 2),
                   DISPLAY_H + 5 - (gear.header.h / 2));
    lv_img_set_pivot(img,
                     gear.header.w/2,
                     gear.header.h/2);

    lv_anim_init(&gear_anim);
    lv_anim_set_var(&gear_anim, img);
    lv_anim_set_exec_cb(&gear_anim, (lv_anim_exec_xcb_t)lv_img_set_angle);

    // angles are in 0.1 degrees (900 = 90°, 3600 = 360°)
    lv_anim_set_values(&gear_anim, 0, 3600);
    lv_anim_set_time(&gear_anim, 10000); // 2 seconds per rotation
    lv_anim_set_repeat_count(&gear_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&gear_anim);

    screen = lv_screen_active();
    s_label = lv_label_create(screen);
    lv_obj_set_width(s_label, 128);
    lv_label_set_text(s_label, "Programming Mode");
    lv_obj_set_style_text_font(s_label, &lv_font_montserrat_10, 0);
    lv_obj_align(s_label, LV_ALIGN_TOP_LEFT, 0, 10);
    // lv_label_set_long_mode(s_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
}

void run_programming_mode() {

    gui_async(gui_build, NULL);

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

    // erase_all_W25Q128J(&memory_chip);

    ESP_LOGI(TAG, "Flash init: %d", stat == true);
    ESP_LOGI(TAG, "Flash Size: 0x%0lx", get_size_W25Q128J(&memory_chip));
    ESP_LOGI(TAG, "Flash ID: 0x%0lx", get_id_W25Q128J(&memory_chip));

    // now the goal is to mount an external parition to this qspi flash
    stat = extfs_setup(&ext_flash_fs, memory_chip.ext_flash, "/ext", "myextfs");
    ESP_LOGI(TAG, "Mount External flash as vfs wl: %d", stat == true);

    fatfs_test();

    ESP_LOGI(TAG, "WL handler: %d\n", ext_flash_fs.wl);
}
