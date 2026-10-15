#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef int esp_err_t;
#define ESP_OK 0
static inline int bsp_display_init(void){return 0;}
static inline void *bsp_lvgl_init(void){return (void*)1;}
static inline bool bsp_lvgl_lock(int ms){(void)ms;return true;}
static inline void bsp_lvgl_unlock(void){}
static inline void bsp_display_backlight(uint8_t p){(void)p;}
