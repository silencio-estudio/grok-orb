/* Movimiento del orb sin LVGL: muelles, curva ease-in-out y transiciones con duración. */
#pragma once
#include <stdbool.h>

/* Muelle amortiguado. Al cambiar el objetivo conserva la velocidad, así que las
 * transiciones entre estados se encadenan sin saltos. */
typedef struct {
    float value, velocity, target;
    float omega;   /* 2π·frecuencia natural (rad/s) */
    float zeta;    /* amortiguamiento: <1 rebota un poco, 1 crítico */
} pet_spring_t;

void pet_spring_init(pet_spring_t *spring, float value, float hz, float zeta);
/* Avanza dt segundos (se recorta a 0,1 s y se integra en pasos de 2 ms). */
void pet_spring_step(pet_spring_t *spring, float dt);

/* --ease-io2 de los tokens de movimiento: cubic-bezier(0.455, 0.03, 0.515, 0.955). */
float pet_ease_io2(float t);

/* Transición de from a to en duration segundos con --ease-io2. */
typedef struct { float from, to, start, duration; } pet_tween_t;
void pet_tween_start(pet_tween_t *tween, float current, float to, float now, float duration);
float pet_tween_value(const pet_tween_t *tween, float now);
bool pet_tween_done(const pet_tween_t *tween, float now);
