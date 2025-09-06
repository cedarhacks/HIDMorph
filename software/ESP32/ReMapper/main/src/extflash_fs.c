#include "extflash_fs.h"

bool extfs_register_partion(ExtFlashFs_t *fs, esp_flash_t *flash, char *parition_name) {
    esp_err_t err = esp_partition_register_external(flash,
                                                    0x0, // parition offset
                                                    flash->size,
                                                    parition_name,
                                                    ESP_PARTITION_TYPE_DATA,
                                                    ESP_PARTITION_SUBTYPE_DATA_FAT,
                                                    &fs->partition);

    return err == ESP_OK;
}

bool extfs_mount_fatfs(ExtFlashFs_t *fs, char *mount_path, char *partition_name) {

    const esp_vfs_fat_mount_config_t mnt = {.format_if_mount_failed = true,
                                            .max_files = 8,
                                            .allocation_unit_size = 4096,
                                            .use_one_fat = false};
    wl_handle_t wl = WL_INVALID_HANDLE;

    esp_err_t stat = esp_vfs_fat_spiflash_mount_rw_wl(mount_path,
                                                      partition_name,
                                                      &mnt,
                                                      &wl);

    fs->wl = wl;
    fs->mounted = stat == ESP_OK;
    return stat == ESP_OK;
}

bool extfs_unmount(ExtFlashFs_t *fs, const char *mount_path) {
    if (!fs || !fs->mounted) return ESP_ERR_INVALID_STATE;

    esp_err_t err = esp_vfs_fat_spiflash_unmount_rw_wl(mount_path, fs->wl);

    if (err == ESP_OK) {
        fs->mounted = false;
        fs->wl = WL_INVALID_HANDLE;
    }

    return err == ESP_OK;
}

bool extfs_setup(ExtFlashFs_t *fs, esp_flash_t *ext_flash, char *mount_path, char *parition_name) {
    bool stat;
    stat = extfs_register_partion(fs, ext_flash, parition_name);
    stat &= extfs_mount_fatfs(fs, mount_path, parition_name);

    return stat == true;
}