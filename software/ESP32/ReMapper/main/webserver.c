#include "webserver.h"

#include <string.h>

void start_access_point() {

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASSWD,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };

    // no auth
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("ReMapper - Started WIFI\n");
}

// -------------------------------------------------------------------------------------

esp_err_t root_get_handler(httpd_req_t *req) {
    const char *resp_str = "<html><body><h1>ReMapper V1</h1></body></html>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    httpd_uri_t uri = {.uri = "/",
                       .method = HTTP_GET,
                       .handler = root_get_handler,
                       .user_ctx = NULL};

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri);
    }

    printf("HTTP server started\n");
}

void webserver_init() {

    start_access_point();
    start_webserver();
}

void webserver_step() {
}
