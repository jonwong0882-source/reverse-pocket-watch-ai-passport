#include "watch_clock.h"
#include "bsp_display.h"
#include "bsp_button.h"
#include "bsp_battery.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lvgl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PAPER 0xF1E7D0
#define INK 0x322B24
#define GOLD 0xB89554
#define RED 0x923C30
#define BG 0x151C20
#define PI 3.14159265358979323846
static watch_clock_t clock_state;
static lv_obj_t *screen, *hands, *date_label, *time_label, *hint, *battery_label;
static lv_obj_t *editor, *edit_value, *edit_field;
static watch_date_t draft;
static bool editing;
static int field, brightness_index = 2;
static int64_t rendered_second = INT64_MIN;
static double seconds_of_day;
static QueueHandle_t inputs;
static const uint8_t brightness[] = {10,20,35,50,70,100};
static const char *fields[] = {"YEAR", "MONTH", "DAY", "HOUR / 24H", "MINUTE", "SECOND"};
typedef struct { bsp_btn_t button; bsp_btn_ev_t event; } input_t;

static lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y, uint32_t color,
                       const lv_font_t *font) {
    lv_obj_t *obj = lv_label_create(parent);
    lv_label_set_text(obj,text);
    lv_obj_set_style_text_font(obj,font,0);
    lv_obj_set_style_text_color(obj,lv_color_hex(color),0);
    lv_obj_set_pos(obj,x,y);
    return obj;
}
static void centered(lv_obj_t *obj, int y) {
    lv_obj_set_width(obj,220);
    lv_obj_set_style_text_align(obj,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_pos(obj,10,y);
}
static lv_obj_t *circle(int x,int y,int diameter,uint32_t fill,uint32_t edge,int border) {
    lv_obj_t *obj=lv_obj_create(screen);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj,x,y);
    lv_obj_set_size(obj,diameter,diameter);
    lv_obj_set_style_radius(obj,LV_RADIUS_CIRCLE,0);
    lv_obj_set_style_bg_color(obj,lv_color_hex(fill),0);
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,0);
    lv_obj_set_style_border_color(obj,lv_color_hex(edge),0);
    lv_obj_set_style_border_width(obj,border,0);
    return obj;
}
static void line(lv_layer_t *layer,float x1,float y1,float x2,float y2,int width,uint32_t color) {
    lv_draw_line_dsc_t d;
    lv_draw_line_dsc_init(&d);
    d.p1=(lv_point_precise_t){x1,y1};
    d.p2=(lv_point_precise_t){x2,y2};
    d.width=width;
    d.color=lv_color_hex(color);
    d.round_start=d.round_end=1;
    lv_draw_line(layer,&d);
}
static void roman(lv_layer_t *layer,const char *text,float cx,float cy) {
    int n=(int)strlen(text);
    float x=cx-(n*7-1)/2.0f, y=cy-7;
    for(int j=0;j<n;j++,x+=7) {
        char c=text[j];
        if(c=='I') {
            line(layer,x+3,y,x+3,y+14,1,INK);
            line(layer,x,y,x+6,y,1,INK);
            line(layer,x,y+14,x+6,y+14,1,INK);
        } else {
            if(c=='X') {
                line(layer,x,y,x+6,y+14,1,INK);
                line(layer,x+6,y,x,y+14,1,INK);
                line(layer,x-1,y+14,x+1,y+14,1,INK);
                line(layer,x+5,y+14,x+7,y+14,1,INK);
            } else {
                line(layer,x,y,x+3,y+14,1,INK);
                line(layer,x+6,y,x+3,y+14,1,INK);
            }
            line(layer,x-1,y,x+1,y,1,INK);
            line(layer,x+5,y,x+7,y,1,INK);
        }
    }
}
static void dial_draw(lv_event_t *event) {
    lv_layer_t *layer=lv_event_get_layer(event);
    for(int i=0;i<60;i++) {
        double a=i*PI/30;
        double inner=i%5==0 ? 85 : 90;
        line(layer,120+sin(a)*inner,143-cos(a)*inner,
             120+sin(a)*95,143-cos(a)*95,i%5==0 ? 2 : 1,INK);
    }
    static const char *numerals[]={"XII","I","II","III","IIII","V","VI","VII","VIII","IX","X","XI"};
    for(int i=0;i<12;i++) {
        double a=i*PI/6;
        roman(layer,numerals[i],120+sin(a)*74,143-cos(a)*74);
    }
}
static void draw_hand(lv_layer_t *layer,double turns,double len,int width,uint32_t color,double tail) {
    double a=turns*2*PI, dx=sin(a),dy=-cos(a);
    line(layer,120-dx*tail,143-dy*tail,120+dx*len,143+dy*len,width,color);
}
static void hands_draw(lv_event_t *event) {
    lv_layer_t *layer=lv_event_get_layer(event);
    draw_hand(layer,seconds_of_day/43200.0,47,5,INK,9);
    draw_hand(layer,seconds_of_day/3600.0,68,3,INK,12);
    draw_hand(layer,seconds_of_day/60.0,80,1,RED,20);
    line(layer,118,143,122,143,7,GOLD);
    line(layer,119,143,121,143,3,RED);
}
static void refresh_editor(void) {
    char text[48];
    snprintf(text,sizeof(text),"%04d-%02d-%02d\n\n%02d : %02d : %02d",
        draft.year,draft.month,draft.day,draft.hour,draft.minute,draft.second);
    lv_label_set_text(edit_value,text);
    lv_label_set_text_fmt(edit_field,"%d / 6   %s",field+1,fields[field]);
}
static void open_editor(void) {
    // Present a forward-time suggestion derived from this session's start anchor.
    int64_t origin=clock_state.origin_us/1000000;
    int64_t elapsed=(esp_timer_get_time()-clock_state.monotonic_us)/1000000;
    draft=watch_date_from_seconds(origin+elapsed);
    field=0;
    editing=true;
    lv_obj_add_flag(time_label,LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(editor,LV_OBJ_FLAG_HIDDEN);
    refresh_editor();
}
static void refresh_clock(lv_timer_t *timer) {
    (void)timer;
    int64_t us=watch_now_us(&clock_state,esp_timer_get_time());
    int64_t rem=us%86400000000LL;
    if(rem<0) rem+=86400000000LL;
    seconds_of_day=rem/1000000.0;
    lv_obj_invalidate(hands);
    int64_t second=us/1000000;
    if(us<0 && us%1000000) --second;
    if(second!=rendered_second) {
        rendered_second=second;
        watch_date_t d=watch_date_from_seconds(second);
        lv_label_set_text_fmt(date_label,"%04d . %02d . %02d",d.year,d.month,d.day);
        lv_label_set_text_fmt(time_label,"%02d : %02d : %02d",d.hour,d.minute,d.second);
    }
    input_t in;
    while(xQueueReceive(inputs,&in,0)==pdTRUE) {
        if(in.event==BSP_BTN_LONG && in.button==BSP_BTN_OK) {
            if(editing) {
                editing=false;
                lv_obj_add_flag(editor,LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(time_label,LV_OBJ_FLAG_HIDDEN);
            }
            else open_editor();
        } else if(in.event==BSP_BTN_CLICK) {
            if(editing) {
                if(in.button==BSP_BTN_UP || in.button==BSP_BTN_DOWN) {
                    watch_adjust(&draft,field,in.button==BSP_BTN_UP ? 1 : -1);
                    refresh_editor();
                } else if(++field==6) {
                    watch_start(&clock_state,watch_date_to_seconds(draft)*1000000,esp_timer_get_time());
                    rendered_second=INT64_MIN;
                    editing=false;
                    lv_obj_remove_flag(time_label,LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(editor,LV_OBJ_FLAG_HIDDEN);
                    lv_label_set_text(hint,"POCKET WATCH DATE");
                } else refresh_editor();
            } else if(in.button==BSP_BTN_UP || in.button==BSP_BTN_DOWN) {
                brightness_index+=in.button==BSP_BTN_UP ? 1 : -1;
                if(brightness_index<0) brightness_index=0;
                if(brightness_index>5) brightness_index=5;
                bsp_display_backlight(brightness[brightness_index]);
            }
        }
    }
}
static void build_screen(void) {
    screen=lv_obj_create(NULL);
    lv_obj_remove_style_all(screen);
    lv_obj_set_style_bg_color(screen,lv_color_hex(BG),0);
    lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);
    label(screen,"RETROGRADE",20,10,GOLD,&lv_font_montserrat_14);
    battery_label=label(screen,"--",190,10,0xA6A79D,&lv_font_montserrat_14);
    // Bow and winding crown, in the same brass as the case.
    circle(107,29,26,BG,GOLD,3);
    circle(112,43,16,GOLD,0xDBBE80,2);
    circle(11,35,218,0x7E6034,0x554125,2);
    circle(15,39,210,GOLD,0xD8BA7D,3);
    circle(21,45,198,PAPER,INK,1);
    lv_obj_t *dial=lv_obj_create(screen);
    lv_obj_remove_style_all(dial);
    lv_obj_set_pos(dial,21,45);
    lv_obj_set_size(dial,198,198);
    lv_obj_add_event_cb(dial,dial_draw,LV_EVENT_DRAW_MAIN,NULL);
    lv_obj_t *brand=label(screen,"TEMPS PERDU",0,0,0x8C7757,&lv_font_montserrat_14);
    lv_obj_set_size(brand,160,18);
    lv_obj_set_style_text_align(brand,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_pos(brand,40,108);
    lv_obj_t *ratio=label(screen,"1 : 1",0,0,0x8C7757,&lv_font_montserrat_14);
    lv_obj_set_pos(ratio,105,172);
    hands=lv_obj_create(screen);
    lv_obj_remove_style_all(hands);
    lv_obj_set_pos(hands,35,58);
    lv_obj_set_size(hands,170,170);
    lv_obj_add_event_cb(hands,hands_draw,LV_EVENT_DRAW_MAIN,NULL);
    lv_obj_t *caption=label(screen,"POCKET WATCH DATE",0,0,GOLD,&lv_font_montserrat_14);
    centered(caption,258);
    date_label=label(screen,"",0,0,PAPER,&lv_font_montserrat_20);
    centered(date_label,276);
    time_label=label(screen,"",0,0,0xA7A79C,&lv_font_montserrat_14);
    centered(time_label,298);
    hint=caption;
    lv_label_set_text(hint,"UNSET - HOLD OK TO SET");
    editor=lv_obj_create(screen);
    lv_obj_set_pos(editor,12,40);
    lv_obj_set_size(editor,216,260);
    lv_obj_set_style_bg_color(editor,lv_color_hex(BG),0);
    lv_obj_set_style_border_color(editor,lv_color_hex(GOLD),0);
    lv_obj_set_style_border_width(editor,1,0);
    lv_obj_set_style_pad_all(editor,10,0);
    lv_obj_remove_flag(editor,LV_OBJ_FLAG_SCROLLABLE);
    label(editor,"SET STARTING TIME",0,0,GOLD,&lv_font_montserrat_14);
    edit_field=label(editor,"",0,35,PAPER,&lv_font_montserrat_14);
    edit_value=label(editor,"",0,68,PAPER,&lv_font_montserrat_20);
    label(editor,"UP / DOWN: change\nOK: next / start\nHOLD OK: cancel",0,165,0xB0A58C,&lv_font_montserrat_14);
    lv_obj_add_flag(editor,LV_OBJ_FLAG_HIDDEN);
    lv_screen_load(screen);
    lv_timer_create(refresh_clock,40,NULL);
    refresh_clock(NULL);
}
static void on_key(bsp_btn_t btn,bsp_btn_ev_t ev,void *ctx) {
    (void)ctx;
    if(ev!=BSP_BTN_CLICK && ev!=BSP_BTN_LONG) return;
    input_t input={btn,ev};
    (void)xQueueSend(inputs,&input,0);
}
void app_main(void) {
    // Monotonic anchor never follows wall-clock corrections. Build time is only
    // an explicitly UNSET suggestion; this offline board needs calibration per boot.
    watch_start(&clock_state,(WATCH_BUILD_EPOCH+8*3600)*1000000,esp_timer_get_time());
    inputs=xQueueCreate(12,sizeof(input_t));
    if(!inputs) { ESP_LOGE("watch","Input queue unavailable"); return; }
    if(bsp_display_init()!=ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE("watch","Display initialization failed"); return;
    }
    bool has_battery=bsp_battery_init()==ESP_OK;
    bool has_buttons=bsp_button_init(on_key,NULL)==ESP_OK;
    if(!bsp_lvgl_lock(2000)) { ESP_LOGE("watch","LVGL lock unavailable"); return; }
    build_screen();
    if(!has_buttons) lv_label_set_text(hint,"BUTTON ERROR - RESTART");
    bsp_lvgl_unlock();
    bsp_display_backlight(brightness[brightness_index]);
    for(;;) {
        int soc=has_battery ? bsp_battery_soc() : -1;
        if(bsp_lvgl_lock(100)) {
            if(soc>=0) lv_label_set_text_fmt(battery_label,"%d%%",soc);
            else lv_label_set_text(battery_label,"--");
            bsp_lvgl_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}
