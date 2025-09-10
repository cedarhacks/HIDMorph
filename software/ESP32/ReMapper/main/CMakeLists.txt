
idf_component_register(
    
    SRCS
        "src/main.c"
        "src/task_spi_usb.c"
        "src/gamepad_hid.c"
        "src/hid_parsing.c"
        "src/webserver.c"
        "src/task_webhost.c"
        "src/programming_mode.c"
        "src/W25Q128J.c"
        "src/extflash_fs.c"
        "src/task_usb_storage_device.c"
        "src/task_gamepad_out.c"
        "src/tusb_descriptors.c"

    INCLUDE_DIRS
        "./include"
        "../../../SHARED/include/"
    
    EMBED_TXTFILES
        "assets/index.html"
        "assets/style.css"
    
    REQUIRES
        tinyusb
        esp_wifi
        nvs_flash
        spi_flash
        esp-tls
        esp_netif
        esp_http_server
)

