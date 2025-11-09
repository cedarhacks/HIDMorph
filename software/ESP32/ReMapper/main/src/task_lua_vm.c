#include <stdatomic.h>
#include "task_lua_vm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
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
#include "task_input_manager.h"
#include "widget_horizontal_select.h"
#include "hid_q.h"

#define MAX_FILE_PATH_LEN (255)

const char *TAG = "LUA_TASK";
static W25Q128J_t memory_chip;
static ExtFlashFs_t ext_flash_fs;

static lv_obj_t *screen_file_select;
char files[20][MAX_FILE_PATH_LEN];
char files_name_only[20][MAX_FILE_PATH_LEN];
char *files_list[20];
char *files_name_only_list[20];

static lv_obj_t *screen_file_running;

int selected_file_i = 0;
int file_count = 0;

static list_selector_t file_list_select;
int global_pin; // assume pin ebvent at a time

typedef enum RUN_MODE {
    MODE_FILE_SELECT,
    MODE_RUN,
} RUN_MODE_t;

static SemaphoreHandle_t mode_sem;
static RUN_MODE_t mode;
static RUN_MODE_t mode_lp;

static volatile bool g_stop = false;

void gui_input(void *pin_ptr) {
    int pin = *(int *)pin_ptr;

    if (pin == BUTTON_NEXT) {
        selected_file_i += 1;

    } else if (pin == BUTTON_PREV) {

        if (mode == MODE_RUN) {
            g_stop = true;
        } else if (mode == MODE_FILE_SELECT) {
            selected_file_i -= 1;
        }

    } else if (pin == BUTTON_SELECT) {
        // enter run mode
        if (xSemaphoreTake(mode_sem, portMAX_DELAY)) {
            mode = MODE_RUN;
            xSemaphoreGive(mode_sem);
        }
    }

    selected_file_i = selected_file_i >= file_count ? 0 : selected_file_i;
    selected_file_i = selected_file_i < 0 ? file_count - 1 : selected_file_i;
    list_selector_scroll_to(&file_list_select, selected_file_i);
}

void clicked_cb(int pin) {
    global_pin = pin;
    gui_async(gui_input, &global_pin);
}

void gui_build_file_select(void *arg) {

    screen_file_select = lv_obj_create(NULL);

    for (int i = 0; i < file_count; i++) {
        files_list[i] = files[i];

        // find last "/" to display
        files_name_only_list[i] = strrchr(files_list[i], '/') + 1;
    }

    file_list_select.items = (char **)files_name_only_list;
    file_list_select.num_items = file_count;
    file_list_select.current_i = 0;
    list_selector_init(screen_file_select, &file_list_select);
    list_selector_scroll_to(&file_list_select, selected_file_i);
}

void gui_build_running(void *arg) {
    screen_file_running = lv_obj_create(NULL);

    lv_obj_t *label = lv_label_create(screen_file_running);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(label, "Running!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void gui_switch_file_select(void *arg) {
    lv_scr_load(screen_file_select);
}

void gui_switch_file_run(void *arg) {
    lv_scr_load(screen_file_running);
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

static void hook_count(lua_State *L, lua_Debug *ar) {
    (void)ar;
    // Cooperatively yield to host task
    lua_yield(L, 0);
}

static int init_hid_mouse(lua_State *L) {

    int result = 1;
    ESP_LOGI(TAG, "from lua: init HID mouse");
    lua_pushnumber(L, result);
    return result;
}

static int set_mouse_pos(lua_State *L) {
    
    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);

    hid_post_mouse(0,x,y,0,0, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(10));
    return 1;
}


void run_lua_file(const char *filename) {

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    lua_register(L, "init_hid_mouse", init_hid_mouse);
    lua_register(L, "set_mouse_pos", set_mouse_pos);
    
    if (luaL_loadfile(L, filename) != LUA_OK) {
        ESP_LOGE(TAG, "load error: %s", lua_tostring(L, -1));
        lua_close(L);
        return;
    }

    lua_State *co = lua_newthread(L);            // Stack: [func, co]
    lua_insert(L, -2);                           // Stack: [co, func]  (put func on top)
    lua_xmove(L, co, 1);                         // L:   [co]
                                                 // co:  [func]
    int co_ref = luaL_ref(L, LUA_REGISTRYINDEX); // pops 'co' from L, keeps it alive

    lua_sethook(co, hook_count, LUA_MASKCOUNT, 10000); // every ~10k VM instr

    for (;;) {
        int nres = 0;
        int rc = lua_resume(co, NULL, 0, &nres); // 5.4 signature

        if (rc == LUA_YIELD) {

            if (g_stop) {
                // Stop further hook interruptions
                lua_sethook(co, NULL, 0, 0);

                (void)lua_resetthread(co);

                luaL_unref(L, LUA_REGISTRYINDEX, co_ref);
                lua_close(L);

                ESP_LOGI(TAG, "STOPPED");
                mode = MODE_FILE_SELECT;
                g_stop = false;
                return;
            }

            vTaskDelay(pdMS_TO_TICKS(1));
            continue;

        } else if (rc == LUA_OK) {

            // finished successfully
            break;

        } else {

            // error on co’s stack
            const char *err = lua_tostring(co, -1);
            ESP_LOGE(TAG, "runtime error: %s", err ? err : "(unknown)");
            break;
        }
    }

    luaL_unref(L, LUA_REGISTRYINDEX, co_ref);
    lua_close(L);

    ESP_LOGI(TAG, "DONE");
    mode = MODE_FILE_SELECT;
}



void task_lua_vm(void *args) {

    mode = MODE_FILE_SELECT;
    mode_lp = mode;
    mode_sem = xSemaphoreCreateMutex();

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
    file_count = list_files("/ext", files, 20);
    ESP_LOGI(TAG, "file count: %d", file_count);

    // // construct the gui after getting a file list
    gui_async(gui_build_file_select, NULL);
    gui_async(gui_build_running, NULL);
    gui_async(gui_switch_file_select, NULL);

    // // register input
    input_listen_click(clicked_cb);

    // necassary to read file names
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // // READ TEST.lua
    // char *current_file = "/ext/TEST.lua";

    while (1) {

        if (mode == MODE_RUN) {
            if (mode_lp == MODE_FILE_SELECT) {
                mode_lp = mode;

                ESP_LOGI(TAG, "SELECTED FILE %s  %s", files_name_only_list[selected_file_i], files_list[selected_file_i]);
                gui_async(gui_switch_file_run, files_name_only_list[selected_file_i]);

                run_lua_file(files_list[selected_file_i]);
                mode = MODE_FILE_SELECT;
            }
        } else if (mode == MODE_FILE_SELECT) {
            if (mode_lp == MODE_RUN) {
                mode_lp = mode;

                gui_async(gui_switch_file_select, NULL);
                ESP_LOGI(TAG, "CHANGING FROM RUN TO FS\n");
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}