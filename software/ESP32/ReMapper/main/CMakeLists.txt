idf_component_register(SRCS "ReMapper.c" 
                            "gamepad_hid.c"
                            "hid_parsing.c"
                            "webserver.c"
                            INCLUDE_DIRS "." "../../../SHARED/include/"
                            REQUIRES 
                            tinyusb
                            esp_wifi
                            nvs_flash
                            esp-tls
                            esp_netif
                            esp_http_server
                            )
