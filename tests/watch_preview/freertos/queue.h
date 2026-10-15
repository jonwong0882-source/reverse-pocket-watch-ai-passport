#pragma once
#include "bsp_button.h"
#include <stddef.h>
#include <string.h>
typedef void *QueueHandle_t;
extern int pending_input;
extern bsp_btn_t queued_button;
extern bsp_btn_ev_t queued_event;
static inline QueueHandle_t xQueueCreate(int n,size_t s){(void)n;(void)s;return (void*)1;}
static inline int xQueueReceive(QueueHandle_t q,void *p,int ticks){
    (void)q;(void)ticks;
    if(!pending_input)return 0;
    memcpy(p,&queued_button,sizeof(queued_button));
    memcpy((char*)p+sizeof(queued_button),&queued_event,sizeof(queued_event));
    pending_input=0;return 1;
}
static inline int xQueueSend(QueueHandle_t q,const void *p,int ticks){
    (void)q;(void)ticks;
    memcpy(&queued_button,p,sizeof(queued_button));
    memcpy(&queued_event,(const char*)p+sizeof(queued_button),sizeof(queued_event));
    pending_input=1;return 1;
}
