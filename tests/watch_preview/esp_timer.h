#pragma once
#include <stdint.h>
extern int64_t simulated_us;
static inline int64_t esp_timer_get_time(void){return simulated_us;}
