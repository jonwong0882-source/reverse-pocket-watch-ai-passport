#include "../../main/watch_main.c"
#include <assert.h>
int64_t simulated_us;
int pending_input;
bsp_btn_t queued_button;
bsp_btn_ev_t queued_event;
static uint16_t pixels[240*320], buffer[240*40];
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *data) {
    uint16_t *src=(uint16_t*)data;
    for(int y=a->y1;y<=a->y2;y++) for(int x=a->x1;x<=a->x2;x++) pixels[y*240+x]=*src++;
    lv_display_flush_ready(d);
}
static void save(const char *name) {
    lv_refr_now(NULL);
    FILE *f=fopen(name,"wb");assert(f);
    fprintf(f,"P6\n240 320\n255\n");
    for(int i=0;i<240*320;i++) {
        uint16_t p=pixels[i];
        unsigned char rgb[]={(unsigned char)((p>>11)*255/31),(unsigned char)(((p>>5)&63)*255/63),(unsigned char)((p&31)*255/31)};
        fwrite(rgb,1,3,f);
    }
    fclose(f);
}
static void key(bsp_btn_t b,bsp_btn_ev_t e) {on_key(b,e,NULL);refresh_clock(NULL);}
int main(void) {
    setbuf(stdout,NULL);
    lv_init();
    lv_display_t *d=lv_display_create(240,320);
    lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(d,buffer,NULL,sizeof(buffer),LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(d,flush);
    inputs=xQueueCreate(12,sizeof(input_t));
    watch_start(&clock_state,watch_date_to_seconds((watch_date_t){2026,10,15,10,9,42})*1000000,0);
    build_screen();
    save("watch-unset.ppm");
    key(BSP_BTN_OK,BSP_BTN_LONG);
    assert(editing && field==0);
    save("watch-settings.ppm");
    key(BSP_BTN_UP,BSP_BTN_CLICK); assert(draft.year==2027);
    key(BSP_BTN_OK,BSP_BTN_LONG); assert(!editing); // Cancel must not change anchor.
    assert(watch_date_from_seconds(clock_state.origin_us/1000000).year==2026);
    key(BSP_BTN_OK,BSP_BTN_LONG);
    for(int i=0;i<6;i++) key(BSP_BTN_OK,BSP_BTN_CLICK);
    assert(!editing && !strcmp(lv_label_get_text(hint),"POCKET WATCH DATE"));
    refresh_clock(NULL);
    lv_label_set_text(battery_label,"82%");
    save("watch-main.ppm");
    simulated_us=1000000;refresh_clock(NULL);save("watch-second.ppm");
    assert(!strcmp(lv_label_get_text(time_label),"10 : 09 : 41"));
    simulated_us=3600000000LL;refresh_clock(NULL);
    assert(!strcmp(lv_label_get_text(time_label),"09 : 09 : 42"));
    save("watch-hour.ppm");
    puts("Actual LVGL screen: PASS (editor, cancel, start, second/hour reverse)");
    watch_start(&clock_state,watch_date_to_seconds((watch_date_t){2024,3,1,0,0,0})*1000000,simulated_us);
    simulated_us+=1000000;refresh_clock(NULL);
    assert(!strcmp(lv_label_get_text(date_label),"2024 . 02 . 29"));
    assert(!strcmp(lv_label_get_text(time_label),"23 : 59 : 59"));
    for(int frame=0;frame<1000;frame++) {
        simulated_us+=40000;refresh_clock(NULL);lv_refr_now(NULL);
    }
    lv_mem_monitor_t usage;
    lv_mem_monitor(&usage);
    printf("LVGL pool: %u bytes free, %u bytes peak used\n",(unsigned)usage.free_size,(unsigned)usage.max_used);
    lv_deinit();
}
