#include "webserver.h"

#include <string.h>

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#include "esp_log.h"


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

#define DNS_PORT 53
#define DNS_BUFFER_SIZE 512
static const char *TAG = "dns_server";

void webserver_init() {

    start_access_point();
    start_webserver();

    struct sockaddr_in server_addr, client_addr;
    socklen_t sock_len = sizeof(client_addr);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0) {
        ESP_LOGE(TAG, "Socket creation failed");
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(DNS_PORT);

    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    ESP_LOGI(TAG, "DNS server started on port %d", DNS_PORT);

    uint8_t buf[DNS_BUFFER_SIZE];

    while (1) {
        int len = recvfrom(sock, buf, DNS_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &sock_len);
        if (len < 0) continue;

        // Craft a minimal DNS response to any query
        // This is a simple fixed format response
        buf[2] |= 0x80; // Set response flag
        buf[3] |= 0x80; // Set recursion available
        buf[7] = 1;     // Answer count

        // Add answer section
        int pos = len;
        buf[pos++] = 0xC0;
        buf[pos++] = 0x0C; // pointer to domain name
        buf[pos++] = 0x00;
        buf[pos++] = 0x01; // type A
        buf[pos++] = 0x00;
        buf[pos++] = 0x01; // class IN
        buf[pos++] = 0x00;
        buf[pos++] = 0x00;
        buf[pos++] = 0x00;
        buf[pos++] = 0x3C; // TTL
        buf[pos++] = 0x00;
        buf[pos++] = 0x04; // data length

        // Set IP: 192.168.4.1 (default ESP32 AP IP)
        buf[pos++] = 192;
        buf[pos++] = 168;
        buf[pos++] = 4;
        buf[pos++] = 1;

        sendto(sock, buf, pos, 0, (struct sockaddr *)&client_addr, sock_len);
    }
}

void webserver_step() {
}
