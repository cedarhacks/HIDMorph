#include "task_lua_vm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "extflash_fs.h"
#include "W25Q128J.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "gui_main.h"
#include "task_display.h"

#define MAX_FILE_PATH_LEN (255)

const char *TAG = "LUA_TASK";
static W25Q128J_t memory_chip;
static ExtFlashFs_t ext_flash_fs;

lv_obj_t *file_screen = NULL;

static void gui_build(void *arg) {
    file_screen = lv_screen_active();
    create_file_pager(file_screen);
}

int list_files(const char *path, char files_paths[][MAX_FILE_PATH_LEN], int file_paths_count) {
    DIR *dir = opendir(path);
    if (!dir) {
        printf("Failed to open directory: %s\n", path);
        return 0;
    }

    char path_as_dir[MAX_FILE_PATH_LEN];
    sprintf(path_as_dir, "%s/", path);
    int strlen_path_as_dir = strlen(path_as_dir);

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < file_paths_count) {

        if (entry->d_type == DT_DIR) {
            continue;

        } else {
            // entry->d_name NOT NULL terminated
            memset(files_paths[count], '\0', MAX_FILE_PATH_LEN);
            strcpy(files_paths[count], path_as_dir);
            // strcat(files_paths[count], entry->d_name);/// ahh no null termination
            memcpy((files_paths[count]) + strlen_path_as_dir,
                   &entry->d_name[0],
                   MAX_FILE_PATH_LEN - strlen_path_as_dir);
            count += 1;
        }
    }
    closedir(dir);

    return count;
}

void task_lua_vm(void *args) {

    gui_async(gui_build, NULL);

    int PIN_NUM_MOSI = 12;
    int PIN_NUM_MISO = 13;
    int PIN_NUM_CLK = 11;
    int PIN_NUM_IO2 = 14;
    int PIN_NUM_IO3 = 21;
    int PIN_NUM_CS = 17;

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

    stat = extfs_setup(&ext_flash_fs, memory_chip.ext_flash, "/ext", "myextfs");
    ESP_LOGI(TAG, "Mount External flash as vfs wl: %d", stat == true);

    // get lua files list
    static char files[20][MAX_FILE_PATH_LEN];
    int file_count = list_files("/ext", files, 20);
    ESP_LOGI(TAG, " --- file count: %d", file_count);

    // char current_file[MAX_FILE_PATH_LEN];
    for (int i = 0; i < file_count; i++) {
        ESP_LOGI(TAG, "file: %s", files[i]);
    }

    // READ TEST.lua
    char *current_file = "/ext/TEST.lua";

    // initialize the LUA vm
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, current_file) == LUA_OK) {
        ESP_LOGI(TAG, "Script ran successfully!");
        if (lua_isnumber(L, -1)) {
            int result = lua_tointeger(L, -1);
            ESP_LOGI(TAG, "Returned value = %d", result);
        }
    } else {
        ESP_LOGE(TAG, "Error: %s", lua_tostring(L, -1));
    }

    while (1) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    lua_close(L);
}