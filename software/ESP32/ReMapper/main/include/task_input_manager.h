#pragma once

#include "button.h"

#define MAX_CLICKED_CALLBACKS (20)
#define MAX_INPUTS (10)

#define BUTTON_NEXT (GPIO_NUM_2)
#define BUTTON_PREV (GPIO_NUM_3)
#define BUTTON_SELECT (GPIO_NUM_18)

typedef void (*CLICKED_CB)(int);


void task_input_manager(void *args);
void input_listen_click( CLICKED_CB cb );