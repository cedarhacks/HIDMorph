#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef void (*gui_cb_t)(void *arg);

typedef struct {
    gui_cb_t cb;
    void *arg;
    SemaphoreHandle_t done; // NULL for async, binary semaphore for sync
} gui_msg_t;


void task_display(void *args);          // init task
bool gui_async(gui_cb_t cb, void *arg); // enqueue and return immediately