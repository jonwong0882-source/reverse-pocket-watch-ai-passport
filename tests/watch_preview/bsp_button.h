#pragma once
#include "bsp_display.h"
typedef enum {BSP_BTN_UP,BSP_BTN_DOWN,BSP_BTN_OK} bsp_btn_t;
typedef enum {BSP_BTN_PRESS,BSP_BTN_CLICK,BSP_BTN_DOUBLE,BSP_BTN_LONG} bsp_btn_ev_t;
static inline int bsp_button_init(void (*cb)(bsp_btn_t,bsp_btn_ev_t,void*),void* p){(void)cb;(void)p;return 0;}
