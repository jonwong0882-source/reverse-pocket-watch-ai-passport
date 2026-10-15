#include "watch_clock.h"
#include <assert.h>
#include <stdio.h>
static void equal(watch_date_t a,watch_date_t b) {
    assert(a.year==b.year && a.month==b.month && a.day==b.day);
    assert(a.hour==b.hour && a.minute==b.minute && a.second==b.second);
}
static watch_date_t back(watch_date_t date,int64_t elapsed) {
    watch_clock_t c;
    watch_start(&c,watch_date_to_seconds(date)*1000000,9000000);
    return watch_date_from_seconds(watch_now_us(&c,9000000+elapsed*1000000)/1000000);
}
int main(void) {
    equal(back((watch_date_t){2026,10,15,13,30,0},3600),(watch_date_t){2026,10,15,12,30,0});
    equal(back((watch_date_t){2026,1,1,0,0,0},1),(watch_date_t){2025,12,31,23,59,59});
    equal(back((watch_date_t){2024,3,1,0,0,0},1),(watch_date_t){2024,2,29,23,59,59});
    equal(back((watch_date_t){2100,3,1,0,0,0},1),(watch_date_t){2100,2,28,23,59,59});
    equal(back((watch_date_t){1970,1,1,0,0,0},1),(watch_date_t){1969,12,31,23,59,59});
    assert(watch_date_to_seconds((watch_date_t){1970,1,1,0,0,0})==0);
    for(int y=1;y<=9999;y+=17) for(int m=1;m<=12;m++) {
        watch_date_t d={y,m,watch_days_in_month(y,m),23,59,59};
        equal(watch_date_from_seconds(watch_date_to_seconds(d)),d);
    }
    watch_clock_t c;
    watch_start(&c,0,100);
    assert(watch_now_us(&c,123556)==-123456); // Subsecond reverse sweep.
    assert(watch_now_us(&c,99)==0);
    assert(watch_now_us(&c,3600000100LL)==-3600000000LL);
    watch_date_t d={2024,2,29,1,2,3};
    watch_adjust(&d,0,1); assert(d.year==2025 && d.day==28);
    watch_adjust(&d,1,1); assert(d.month==3 && d.day==28);
    watch_adjust(&d,5,-1); assert(d.second==2);
    assert(!watch_date_valid((watch_date_t){2025,2,29,0,0,0}));
    equal(back((watch_date_t){1,1,1,0,0,0},86400),(watch_date_t){1,1,1,0,0,0});
    puts("Reverse clock: PASS (1:1, subsecond, leap/century, pre-epoch, editing, bounds)");
}
