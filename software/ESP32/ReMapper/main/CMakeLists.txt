# Text assets (served as strings)
file(GLOB_RECURSE SITE_ASSETS_TXT
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.html"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.htm"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.css"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/robots.txt"
)

# Binary assets (serve as bytes)
file(GLOB_RECURSE SITE_ASSETS_BIN
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.jpg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.jpeg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.gif"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.ico"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.webp"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.woff"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.woff2"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.otf"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets_embed/*.wasm"
)

message(STATUS "Embedding txt assets : ${SITE_ASSETS_TXT}")
message(STATUS "Embedding bin assets : ${SITE_ASSETS_BIN}")

idf_component_register(
    SRCS "ReMapper.c" "gamepad_hid.c" "hid_parsing.c" "webserver.c"
    INCLUDE_DIRS "." "../../../SHARED/include/"
    EMBED_TXTFILES ${SITE_ASSETS_TXT}
    REQUIRES tinyusb esp_wifi nvs_flash esp-tls esp_netif esp_http_server
)