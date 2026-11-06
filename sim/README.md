# Simulador de escritorio del orb

Ventana de 466×466 con máscara redonda, igual que la pantalla de la Waveshare ESP32-S3-Touch-AMOLED-1.75C. Ejecuta **la misma interfaz del firmware**: `firmware/pet/components/pet_ui/pet_ui.c` y `pet_core/pet_core.c` se compilan sin cambios, con LVGL 9.5.0 (la versión del firmware) y una placa simulada. No hace falta la placa ni ESP-IDF, y no se flashea nada.

![Estado «Lista» en el simulador](../docs/sim/2-lista.png)

## Lanzarlo

Requisitos: macOS, Node 22, las Xcode Command Line Tools (`cc`) y SDL2:

```sh
brew install sdl2
npm run sim
```

La primera vez descarga LVGL `v9.5.0` en `.build/sim/lvgl` (fijado al commit `85aa60d`) y compila unos 470 archivos. Después solo recompila lo que cambia.

| Orden | Qué hace |
|---|---|
| `npm run sim` | Abre la ventana |
| `npm run sim:shots` | Regenera las capturas de `docs/sim/` sin abrir ventana |
| `npm run sim:test` | Prueba sin ventana: pasa por el mismo código que las teclas (`B`, `1`–`8`, `←`/`→`, `Espacio`) y comprueba cada estado de `pet_core` |
| `node sim/sim.mjs --shots <carpeta>` | Capturas en otra carpeta |
| `node sim/sim.mjs --build` | Solo compila (`.build/sim/grok-orb-sim`) |

## Controles

| Tecla o gesto | Acción |
|---|---|
| `1` … `8` | Ir a un estado (tabla de abajo) |
| `←` / `→` o `Espacio` | Estado anterior / siguiente |
| `B` (mantener) | Botón BOOT: mantener para hablar y soltar para enviar. Un bot simulado «piensa» 1,2 s y «habla» 5 s |
| `S` | Captura del estado actual en `.build/sim/shots/` |
| `Q` o `Esc` | Salir |
| Clic en el orb | Caricia |
| Arrastrar hacia arriba o clic en la barra inferior | Abrir Ajustes |

El ratón hace de dedo: los deslizadores, interruptores y botones de Ajustes y de «Conexión y red» funcionan como en la placa.

## Estados

| # | Estado | Cómo se llega en el simulador | Captura |
|---|---|---|---|
| 1 | Conectando | Enlace con el puente en `CONNECTING` | [1-conectando.png](../docs/sim/1-conectando.png) |
| 2 | Lista | Enlace `READY`, en reposo | [2-lista.png](../docs/sim/2-lista.png) |
| 3 | Escuchando | BOOT pulsado | [3-escuchando.png](../docs/sim/3-escuchando.png) |
| 4 | Pensando | BOOT soltado, esperando respuesta | [4-pensando.png](../docs/sim/4-pensando.png) |
| 5 | Hablando | Audio de respuesta reproduciéndose, con subtítulo | [5-hablando.png](../docs/sim/5-hablando.png) |
| 6 | Sin conexión | Sin Wi-Fi ni puente (`OFFLINE`). Como en la placa, el orb se pone gris y dice «Sin conexión» tras 10 s seguidos sin enlace; antes muestra «Conectando la voz…» | [6-sin-conexion.png](../docs/sim/6-sin-conexion.png) |
| 7 | Ajustes | Página de ajustes | [7-ajustes.png](../docs/sim/7-ajustes.png) |
| 8 | Conexión y red | Página de conexión | [8-conexion-y-red.png](../docs/sim/8-conexion-y-red.png) |

Los estados no son decorados: el simulador manda a `pet_core` los mismos eventos que el firmware (`PET_EV_LINK`, `PRESS`, `RELEASE`, `BUFFERED`, `PLAYING`…). Así que la cara, el parpadeo, el movimiento al hablar y los textos salen del código real. La placa se trata como alimentada por USB (decisión D9).

## Cómo está hecho

| Archivo | Papel |
|---|---|
| `sim_main.c` | Estados, teclado, bot simulado, capturas PNG (RGB565 → RGBA con la máscara redonda) |
| `fakes/` | Sustitutos mínimos de `board_port.h`, FreeRTOS (un solo hilo), `esp_timer`, USB y mbedtls para que `pet_ui.c` compile tal cual |
| `lv_conf.h` | LVGL con la configuración del firmware: color de 16 bits, Montserrat 14/16/20/28, snapshot y driver SDL |
| `sim.mjs` | Descarga LVGL, compila en paralelo, enlaza con SDL2 y lanza |

Diferencias con la placa:

- **Brillo:** la ventana lo imita con una capa negra semitransparente. Las capturas de `docs/sim/` muestran los píxeles que recibe el panel, sin aplicar el brillo.
- **Tiempo:** en las capturas es virtual (1,4 s tras entrar en cada estado), así que salen idénticas en cada ejecución. En la ventana es tiempo real.
- **No se simulan** el audio, la red, el Bluetooth ni la NVS. Los textos de estado de red son los mismos que escribe `pet_app.c`.
- El volcado por USB de `pet.snapshot` se descarta; para capturas usa `S` o `--shots`.
