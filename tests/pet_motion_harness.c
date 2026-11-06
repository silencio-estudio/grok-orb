#include "pet_motion.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void run(pet_spring_t *s, float seconds) { for (float t = 0; t < seconds; t += 1 / 60.0f) pet_spring_step(s, 1 / 60.0f); }

int main(void)
{
    /* Curva: extremos, simetría de ease-in-out y monotonía. */
    assert(pet_ease_io2(0) == 0 && pet_ease_io2(1) == 1 && pet_ease_io2(-1) == 0 && pet_ease_io2(2) == 1);
    assert(fabsf(pet_ease_io2(0.5f) - 0.515f) < 0.005f);   /* la curva no es del todo simétrica */
    assert(pet_ease_io2(0.1f) < 0.05f && pet_ease_io2(0.9f) > 0.95f);
    for (float x = 0, prev = 0; x <= 1; x += 0.01f) { float y = pet_ease_io2(x); assert(y >= prev - 1e-5f); prev = y; }

    /* Muelle: llega al objetivo, con poco rebote y sin explotar con pasos grandes. */
    pet_spring_t s; pet_spring_init(&s, 0, 2, 0.7f); s.target = 1;
    float peak = 0; for (float t = 0; t < 3; t += 1 / 60.0f) { pet_spring_step(&s, 1 / 60.0f); if (s.value > peak) peak = s.value; }
    assert(fabsf(s.value - 1) < 0.005f && peak < 1.1f && peak > 1.0f);
    pet_spring_init(&s, 0, 9, 0.85f); s.target = 5; pet_spring_step(&s, 10); assert(isfinite(s.value) && fabsf(s.value - 5) < 0.5f);
    pet_spring_step(&s, 0); pet_spring_step(&s, -1); assert(isfinite(s.value));

    /* Encadenado: cambiar el objetivo a mitad de camino conserva la velocidad. */
    pet_spring_init(&s, 0, 2, 0.7f); s.target = 1; run(&s, 0.15f);
    float v = s.velocity, x = s.value; s.target = -1; pet_spring_step(&s, 0.002f);
    assert(v > 0 && fabsf(s.velocity - v) < 0.2f * fabsf(v) && s.value >= x);
    pet_spring_init(&s, 0, 2, 1); s.target = 1; run(&s, 4); assert(fabsf(s.value - 1) < 0.001f);

    /* Transición con duración. */
    pet_tween_t tw; pet_tween_start(&tw, 0, 10, 1, 0.5f);
    assert(pet_tween_value(&tw, 1) == 0 && fabsf(pet_tween_value(&tw, 1.25f) - 5.15f) < 0.05f && pet_tween_value(&tw, 2) == 10);
    assert(!pet_tween_done(&tw, 1.4f) && pet_tween_done(&tw, 1.5f));
    pet_tween_start(&tw, 3, 7, 0, 0); assert(pet_tween_value(&tw, 0) == 7);
    pet_tween_start(&tw, 0, 1, 5, 1); assert(pet_tween_value(&tw, 4) == 1);   /* reloj que vuelve atrás */
    puts("PET_MOTION_OK");
    return 0;
}
