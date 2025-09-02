
idf_component_register(
    SRCS "ReMapper.c" "gamepad_hid.c" "hid_parsing.c" "webserver.c"
    INCLUDE_DIRS "." "../../../SHARED/include/"
    EMBED_TXTFILES "assets/index.html" "assets/style.css"
    REQUIRES tinyusb esp_wifi nvs_flash esp-tls esp_netif esp_http_server
)

# idf_component_register(
#     SRCS "test.c"
#     EMBED_TXTFILES "index.html"
# )