#include "watch_clock.h"
#include <stddef.h>
#define MIN_US (-62135596800000000LL)
#define MAX_US (253402300799999999LL)
int watch_days_in_month(int y, int m) {
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if(m < 1 || m > 12) return 0;
    return days[m-1] + (m == 2 && y%4 == 0 && (y%100 != 0 || y%400 == 0));
}
bool watch_date_valid(watch_date_t d) {
    return d.year >= 1 && d.year <= 9999 && d.month >= 1 && d.month <= 12 &&
        d.day >= 1 && d.day <= watch_days_in_month(d.year,d.month) &&
        d.hour >= 0 && d.hour < 24 && d.minute >= 0 && d.minute < 60 && d.second >= 0 && d.second < 60;
}
int64_t watch_date_to_seconds(watch_date_t d) {
    int y = d.year - (d.month <= 2);
    int era = (y >= 0 ? y : y-399)/400;
    unsigned yo = (unsigned)(y-era*400);
    unsigned doy = (153*(d.month+(d.month>2 ? -3 : 9))+2)/5+d.day-1;
    int64_t days = era*146097LL+yo*365+yo/4-yo/100+doy-719468;
    return days*86400 + d.hour*3600 + d.minute*60 + d.second;
}
watch_date_t watch_date_from_seconds(int64_t seconds) {
    int64_t days = seconds/86400, rem = seconds%86400;
    if(rem < 0) { rem += 86400; --days; }
    int64_t z = days+719468;
    int era = (int)((z >= 0 ? z : z-146096)/146097);
    unsigned doe = (unsigned)(z-era*146097LL);
    unsigned yo = (doe-doe/1460+doe/36524-doe/146096)/365;
    int y = (int)yo+era*400;
    unsigned doy = doe-(365*yo+yo/4-yo/100);
    unsigned mp = (5*doy+2)/153;
    int day = (int)(doy-(153*mp+2)/5+1);
    int month = (int)mp+(mp<10 ? 3 : -9);
    return (watch_date_t){y+(month<=2),month,day,(int)(rem/3600),(int)(rem%3600/60),(int)(rem%60)};
}
void watch_start(watch_clock_t *c, int64_t origin, int64_t mono) {
    c->origin_us = origin < MIN_US ? MIN_US : origin > MAX_US ? MAX_US : origin;
    c->monotonic_us = mono;
}
int64_t watch_now_us(const watch_clock_t *c, int64_t mono) {
    // ESP monotonic time is nonnegative; a backwards input never advances history.
    int64_t elapsed = mono > c->monotonic_us ? mono-c->monotonic_us : 0;
    return elapsed > c->origin_us-MIN_US ? MIN_US : c->origin_us-elapsed;
}
void watch_adjust(watch_date_t *d, int field, int delta) {
    int *values[] = {&d->year,&d->month,&d->day,&d->hour,&d->minute,&d->second};
    if(field<0 || field>5) return;
    int lo[] = {1,1,1,0,0,0};
    int hi[] = {9999,12,watch_days_in_month(d->year,d->month),23,59,59};
    int v = *values[field]+delta;
    *values[field] = v<lo[field] ? hi[field] : v>hi[field] ? lo[field] : v;
    int max = watch_days_in_month(d->year,d->month);
    if(d->day>max) d->day=max;
}
