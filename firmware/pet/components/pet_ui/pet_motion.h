/* Movimiento del orb sin LVGL: muelles, curvas ease-in-out y transiciones con duración. */
#pragma once
#include <stdbool.h>

/* Muelle amortiguado. Al cambiar el objetivo conserva la velocidad, así que las
 * transiciones entre estados se encadenan sin saltos. Con zeta = 1 (crítico) no rebota
 * y, partiendo de reposo, recorre el camino en forma de S: arranca y frena suave. */
typedef struct {
    float value, velocity, target;
    float omega;   /* 2π·frecuencia natural (rad/s) */
    float zeta;    /* amortiguamiento: <1 rebota, 1 crítico */
} pet_spring_t;

void pet_spring_init(pet_spring_t *spring, float value, float hz, float zeta);
/* Muelle crítico que llega al 95 % del camino en settle segundos (4,74/ω). */
void pet_spring_init_settle(pet_spring_t *spring, float value, float settle);
/* Avanza dt segundos (se recorta a 0,1 s y se integra en pasos de 2 ms). */
void pet_spring_step(pet_spring_t *spring, float dt);
/* En reposo: cerca del objetivo y casi sin velocidad. */
bool pet_spring_settled(const pet_spring_t *spring, float tolerance);

/* Curvas de los tokens de movimiento (cubic-bezier). */
typedef enum {
    PET_EASE_IO1,   /* --ease-io1 Sine:  (0.445, 0.05, 0.55, 0.95) */
    PET_EASE_IO2,   /* --ease-io2 Quad:  (0.455, 0.03, 0.515, 0.955) */
    PET_EASE_IO3,   /* --ease-io3 Cubic: (0.645, 0.045, 0.355, 1) */
} pet_ease_t;
float pet_ease(pet_ease_t curve, float t);
float pet_ease_io2(float t);

/* Transición de from a to en duration segundos con una de esas curvas. */
typedef struct { float from, to, start, duration; pet_ease_t curve; } pet_tween_t;
void pet_tween_start(pet_tween_t *tween, float current, float to, float now, float duration);
void pet_tween_start_curve(pet_tween_t *tween, pet_ease_t curve, float current, float to, float now, float duration);
float pet_tween_value(const pet_tween_t *tween, float now);
bool pet_tween_done(const pet_tween_t *tween, float now);
