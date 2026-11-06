#pragma once
#include "FreeRTOS.h"
typedef void *SemaphoreHandle_t;
static int sim_semaphore;
static inline SemaphoreHandle_t xSemaphoreCreateMutex(void){return &sim_semaphore;}
static inline SemaphoreHandle_t xSemaphoreCreateBinary(void){return &sim_semaphore;}
#define xSemaphoreTake(s,t) ((void)(s),(void)(t),pdTRUE)
#define xSemaphoreGive(s) ((void)(s),pdTRUE)
