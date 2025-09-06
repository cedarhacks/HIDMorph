#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/spi_master.h"
#include "esp_flash.h"
#include "esp_partition.h"
#include "wear_levelling.h"

#include "esp_random.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"



typedef struct ExtFlashFs {
    const esp_partition_t *partition;
    bool mounted;
    wl_handle_t wl; // wear leveling hadnle
} ExtFlashFs_t;

bool extfs_register_partion(ExtFlashFs_t *fs, esp_flash_t *flash, char *parition_name);
bool extfs_mount_fatfs(ExtFlashFs_t *fs, char *mount_path, char *partition_name);
bool extfs_unmount(ExtFlashFs_t *fs, const char *mount_path);