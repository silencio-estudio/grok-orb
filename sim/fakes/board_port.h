/* Placa simulada: solo lo que pet_ui.c usa de board_port.h. */
#pragma once
#include <stdbool.h>
#include "lvgl.h"
#include "esp_err.h"
typedef struct {
    bool (*display_lock)(int timeout_ms);
    void (*display_unlock)(void);
    void (*set_brightness)(int pct);
    void (*panel_sleep)(bool sleep);
} sim_board_t;
const sim_board_t *pet_board(void);
lv_display_t *pet_board_display_start(lv_indev_t **touch);
