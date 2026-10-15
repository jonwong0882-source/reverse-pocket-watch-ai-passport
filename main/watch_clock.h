#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct { int year, month, day, hour, minute, second; } watch_date_t;
typedef struct { int64_t origin_us, monotonic_us; } watch_clock_t;
int watch_days_in_month(int year, int month);
bool watch_date_valid(watch_date_t date);
int64_t watch_date_to_seconds(watch_date_t date);
watch_date_t watch_date_from_seconds(int64_t seconds);
int64_t watch_now_us(const watch_clock_t *clock, int64_t monotonic_us);
void watch_start(watch_clock_t *clock, int64_t origin_us, int64_t monotonic_us);
void watch_adjust(watch_date_t *date, int field, int delta);
