#include "pet_motion.h"
#include <math.h>

void pet_spring_init(pet_spring_t *s, float value, float hz, float zeta)
{
    *s = (pet_spring_t){ .value = value, .target = value, .omega = 6.2831853f * hz, .zeta = zeta };
}

void pet_spring_step(pet_spring_t *s, float dt)
{
    if (!(dt > 0)) return;
    if (dt > 0.1f) dt = 0.1f;   /* tras una pausa larga no se dispara */
    /* Euler semiimplícito en pasos de 2 ms: estable con ω·h < 0,2 (hasta ~16 Hz). */
    while (dt > 0) {
        float h = dt > 0.002f ? 0.002f : dt;
        float a = s->omega * s->omega * (s->target - s->value) - 2 * s->zeta * s->omega * s->velocity;
        s->velocity += a * h;
        s->value += s->velocity * h;
        dt -= h;
    }
}

static float bezier(float t, float p1, float p2)
{
    float u = 1 - t;
    return 3 * u * u * t * p1 + 3 * u * t * t * p2 + t * t * t;
}

float pet_ease_io2(float x)
{
    const float x1 = 0.455f, y1 = 0.03f, x2 = 0.515f, y2 = 0.955f;
    if (x <= 0) return 0;
    if (x >= 1) return 1;
    /* Bisección sobre x(t), monótona en [0,1]: 24 pasos dan error < 1e-7. */
    float lo = 0, hi = 1, t = x;
    for (int i = 0; i < 24; i++) {
        t = (lo + hi) / 2;
        if (bezier(t, x1, x2) < x) lo = t; else hi = t;
    }
    return bezier(t, y1, y2);
}

void pet_tween_start(pet_tween_t *tw, float current, float to, float now, float duration)
{
    *tw = (pet_tween_t){ .from = current, .to = to, .start = now, .duration = duration };
}

float pet_tween_value(const pet_tween_t *tw, float now)
{
    if (tw->duration <= 0 || now < tw->start) return tw->to;
    float p = (now - tw->start) / tw->duration;
    return tw->from + (tw->to - tw->from) * pet_ease_io2(p);
}

bool pet_tween_done(const pet_tween_t *tw, float now)
{
    return now - tw->start >= tw->duration;
}
