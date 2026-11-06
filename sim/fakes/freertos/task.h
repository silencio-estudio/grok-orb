#pragma once
#include "FreeRTOS.h"
typedef void *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);
/* La tarea de arranque de la interfaz se ejecuta en el acto, en el mismo hilo. */
static inline BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn,const char *name,uint32_t stack,void *arg,int priority,TaskHandle_t *handle,int core){
    (void)name;(void)stack;(void)priority;(void)core;if(handle)*handle=0;fn(arg);return pdPASS;
}
static inline void vTaskDelete(TaskHandle_t task){(void)task;}
