/* Configuración de LVGL para el simulador de escritorio. Sigue la del firmware
 * (firmware/pet/sdkconfig.defaults): color de 16 bits, Montserrat 14/16/20/28 y
 * snapshot. Lo demás queda con los valores por defecto de LVGL 9.5. */
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB
#define LV_USE_OS LV_OS_NONE
/* 60 fps en el Mac; la placa sigue con 33 ms (CONFIG_LV_DEF_REFR_PERIOD por defecto). */
#define LV_DEF_REFR_PERIOD 16
#define LV_DPI_DEF 130

#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#define LV_USE_SNAPSHOT 1
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

#define LV_USE_SDL 1
#define LV_SDL_INCLUDE_PATH <SDL2/SDL.h>
#define LV_SDL_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
#define LV_SDL_BUF_COUNT 1
#define LV_SDL_ACCELERATED 1
#define LV_SDL_FULLSCREEN 0
#define LV_SDL_DIRECT_EXIT 1
#define LV_SDL_MOUSEWHEEL_MODE LV_SDL_MOUSEWHEEL_MODE_ENCODER

#define LV_BUILD_EXAMPLES 0
#define LV_BUILD_DEMOS 0
#endif
