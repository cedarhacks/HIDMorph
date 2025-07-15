#pragma once

#include "esp_wifi.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_http_server.h"
#include "esp_log.h"


#define AP_SSID "ReMapperV1"
#define AP_PASSWD "password"

void start_access_point();

void webserver_init();
void webserver_step();
