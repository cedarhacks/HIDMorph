#include <stdatomic.h>
#include "task_lua_vm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "extflash_fs.h"
#include "W25Q128J.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

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
#define ORIGINAL_IO_OPEN_KEY "my_original_io_open"

typedef enum RUN_MODE {
    MODE_FILE_SELECT,
    MODE_RUN,
} RUN_MODE_t;

const char *TAG = "LUA_TASK";
static W25Q128J_t memory_chip;
static ExtFlashFs_t ext_flash_fs;

static lv_obj_t *screen_file_select;
char files[20][MAX_FILE_PATH_LEN];
char files_name_only[20][MAX_FILE_PATH_LEN];
char *files_list[20];
char *files_name_only_list[20];

int selected_file_i = 0;
int file_count = 0;

static list_selector_t file_list_select;
int global_pin; // assume pin ebvent at a time

static lv_obj_t *screen_file_running;
lv_obj_t *running_label;

static SemaphoreHandle_t mode_sem;
static RUN_MODE_t mode;
static RUN_MODE_t mode_lp;

static volatile bool g_stop = false;
static const char *OVERRIDE_FILE_PREFIX = "/ext/user_data/";

lua_State *L = NULL;
int L_good = 0;
static int keyboard_callback_ref = LUA_NOREF;
static int mouse_callback_ref = LUA_NOREF;

// ---------------------------------------------------------------------------------------------------------

static int register_keyboard_callback(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    if (keyboard_callback_ref != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, keyboard_callback_ref);
        keyboard_callback_ref = LUA_NOREF;
    }

    // Store the new function
    lua_pushvalue(L, 1);
    keyboard_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

static int register_mouse_callback(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    if (mouse_callback_ref != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, mouse_callback_ref);
        mouse_callback_ref = LUA_NOREF;
    }

    // Store the new function
    lua_pushvalue(L, 1);
    mouse_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

void trigger_keyboard_event(int keycode, bool pressed) {
    if (!L_good || L == NULL)
        return;

    if (keyboard_callback_ref == LUA_NOREF)
        return;

    // Push the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, keyboard_callback_ref);

    // Push arguments
    lua_pushinteger(L, keycode);
    lua_pushboolean(L, pressed);

    // Call function with 2 args, 0 return values
    if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
        printf("Lua keyboard callback error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

void trigger_mouse_event(int buttons, int8_t dx, int8_t dy, int8_t wheel) {
    if (!L_good || L == NULL)
        return;

    if (mouse_callback_ref == LUA_NOREF)
        return;

    // Push the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, mouse_callback_ref);

    // Push arguments
    lua_pushinteger(L, buttons);
    lua_pushinteger(L, dx);
    lua_pushinteger(L, dy);
    lua_pushinteger(L, wheel);

    // Call function with 2 args, 0 return values
    if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
        printf("Lua mouse callback error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

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

    lv_obj_t *loader = lv_spinner_create(screen_file_running);
    lv_obj_set_size(loader, 10, 10);
    lv_obj_set_pos(loader, 4, 4);
    lv_obj_set_style_anim_time(loader, 1200, LV_PART_MAIN);   // speed
    lv_obj_set_style_arc_width(loader, 1, LV_PART_INDICATOR); // line thickness

    running_label = lv_label_create(screen_file_running);
    lv_label_set_long_mode(running_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(running_label, &lv_font_montserrat_10, 0);
    lv_label_set_text_fmt(running_label, "Running");
    lv_obj_align(running_label, LV_ALIGN_CENTER, 0, 0);
}

void gui_set_run_text(void *arg) {
    char *text = (char *)arg;
    lv_label_set_text_fmt(running_label, text);
}

void gui_switch_file_select(void *arg) {
    lv_scr_load(screen_file_select);
}

void gui_switch_file_run(void *arg) {
    lv_label_set_text_fmt(running_label, "Running");
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

static int wait_ms_lua(lua_State *L) {
    double ms = luaL_checknumber(L, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    return 1;
}

static int get_time_ms_lua(lua_State *L) {
    float ms = esp_timer_get_time() / 1000.0f;
    lua_pushnumber(L, (lua_Number)ms);
    return 1;
}

static int set_mouse_pos(lua_State *L) {
    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);
    hid_post_mouse(&output_events_q, 0, x, y, 0, 0, 0);
    // vTaskDelay(pdMS_TO_TICKS(10));
    return 1;
}

static int set_mouse_raw(lua_State *L) {
    int buttons = luaL_checknumber(L, 1);
    int dx = luaL_checknumber(L, 2);
    int dy = luaL_checknumber(L, 3);
    int wheel = luaL_checknumber(L, 4);
    hid_post_mouse(&output_events_q, buttons, dx, dy, wheel, 0, 0);
    // vTaskDelay(pdMS_TO_TICKS(20));
    return 1;
}

static int display_set_text(lua_State *L) {
    char *text = luaL_checkstring(L, 1);
    gui_sync(gui_set_run_text, text, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(1));
    return 1;
}

static int override_io_open(lua_State *L) {
    // Get original args
    const char *filename = luaL_checkstring(L, 1);
    const char *mode = luaL_optstring(L, 2, "r");

    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s", OVERRIDE_FILE_PREFIX, filename);

    // Clear stack & call the original io.open from the registry
    lua_settop(L, 0);

    // Push original io.open
    lua_getfield(L, LUA_REGISTRYINDEX, ORIGINAL_IO_OPEN_KEY);
    if (!lua_isfunction(L, -1)) {
        return luaL_error(L, "original io.open not found");
    }

    // Push new arguments: fullpath, mode
    lua_pushstring(L, fullpath);
    lua_pushstring(L, mode);

    // Call original io.open(fullpath, mode)
    lua_call(L, 2, 1);

    // One return value (whatever io.open returned)
    return 1;
}

static int register_override_io_open(lua_State *L) {
    // Get io table
    lua_getglobal(L, "io"); // stack: io

    // Save original io.open in registry
    lua_getfield(L, -1, "open");                              // stack: io, io.open
    lua_setfield(L, LUA_REGISTRYINDEX, ORIGINAL_IO_OPEN_KEY); // registry[KEY] = io.open

    // Set our override as io.open
    lua_pushcfunction(L, override_io_open); // stack: io, override
    lua_setfield(L, -2, "open");            // io.open = override

    lua_pop(L, 1); // pop io table

    return 0; // no Lua return values
}

void lua_register_icons(lua_State *L) {
    // Create ICON = {}
    lua_newtable(L);

#define ICON(name, sym)     \
    lua_pushstring(L, sym); \
    lua_setfield(L, -2, name)

    // Core icons
    ICON("OK", LV_SYMBOL_OK);
    ICON("CLOSE", LV_SYMBOL_CLOSE);
    ICON("WARNING", LV_SYMBOL_WARNING);
    ICON("CHARGE", LV_SYMBOL_CHARGE);
    ICON("POWER", LV_SYMBOL_POWER);

    // Connectivity
    ICON("WIFI", LV_SYMBOL_WIFI);
    ICON("BLUETOOTH", LV_SYMBOL_BLUETOOTH);
    ICON("USB", LV_SYMBOL_USB);

    // Media
    ICON("AUDIO", LV_SYMBOL_AUDIO);
    ICON("VIDEO", LV_SYMBOL_VIDEO);
    ICON("PLAY", LV_SYMBOL_PLAY);
    ICON("PAUSE", LV_SYMBOL_PAUSE);
    ICON("STOP", LV_SYMBOL_STOP);

    // Navigation arrows
    ICON("UP", LV_SYMBOL_UP);
    ICON("DOWN", LV_SYMBOL_DOWN);
    ICON("LEFT", LV_SYMBOL_LEFT);
    ICON("RIGHT", LV_SYMBOL_RIGHT);

    // Battery
    ICON("BATTERY_EMPTY", LV_SYMBOL_BATTERY_EMPTY);
    ICON("BATTERY_1", LV_SYMBOL_BATTERY_1);
    ICON("BATTERY_2", LV_SYMBOL_BATTERY_2);
    ICON("BATTERY_3", LV_SYMBOL_BATTERY_3);
    ICON("BATTERY_FULL", LV_SYMBOL_BATTERY_FULL);

    // File / system
    ICON("TRASH", LV_SYMBOL_TRASH);
    ICON("EDIT", LV_SYMBOL_EDIT);
    ICON("SAVE", LV_SYMBOL_SAVE);
    ICON("REFRESH", LV_SYMBOL_REFRESH);
    ICON("SETTINGS", LV_SYMBOL_SETTINGS);
    ICON("HOME", LV_SYMBOL_HOME);
    ICON("DOWNLOAD", LV_SYMBOL_DOWNLOAD);

#undef ICON

    lua_setglobal(L, "ICON");
}

void handle_hid_inputs() {
    hid_evt_t e;

    int max_per_call = 16;
    int i = 0;

    if (xQueueReceive(input_events_q, &e, 0) != pdTRUE) return;

    switch (e.kind) {
    case HID_EVT_KEYBOARD:
        // tud_hid_keyboard_report(REPORT_ID_KBD, e.u.kbd.mods, e.u.kbd.keycodes);
        trigger_keyboard_event(e.u.kbd.keycodes[0], !e.u.kbd.is_released);
        break;

    case HID_EVT_MOUSE:
        // ESP_LOGI(TAG, "b:%d x:%d y:%d w:%d", e.u.mouse.buttons, e.u.mouse.x, e.u.mouse.y, e.u.mouse.wheel);
        trigger_mouse_event(e.u.mouse.buttons, e.u.mouse.x, e.u.mouse.y, e.u.mouse.wheel);
        break;

    case HID_EVT_GAMEPAD:
        // Use generic sender for your packed struct
        // tud_hid_report(REPORT_ID_GAMEPAD, &e.u.gp, sizeof(e.u.gp));
        break;
    }
}

void run_lua_file(const char *filename) {

    L = luaL_newstate();
    luaL_openlibs(L);

    // register functionality
    //   functions lua can call
    lua_register(L, "init_hid_mouse", init_hid_mouse);
    lua_register(L, "set_mouse_pos", set_mouse_pos);
    lua_register(L, "set_mouse_raw", set_mouse_raw);
    lua_register(L, "display_set_text", display_set_text);
    lua_register(L, "wait_ms", wait_ms_lua);
    lua_register(L, "get_time_ms", get_time_ms_lua);
    lua_register_icons(L);

    //   callbacks
    lua_register(L, "register_keyboard_callback", register_keyboard_callback);
    lua_register(L, "register_mouse_callback", register_mouse_callback);

    // override functions
    register_override_io_open(L); // broken

    L_good = 1;

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

    // lua_sethook(co, hook_count, LUA_MASKCOUNT, 10000); // every ~10k VM instr
    lua_sethook(co, hook_count, LUA_MASKCOUNT, 100); // every ~10k VM instr

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

            handle_hid_inputs();

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
    L_good = 0;

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