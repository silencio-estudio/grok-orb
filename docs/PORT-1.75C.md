# Plan de port: Waveshare ESP32-S3-Touch-AMOLED-1.43C → 1.75C

Estado: **solo planificación**. Este documento no cambia firmware, herramientas ni puente. Ninguna orden de este plan se ha ejecutado contra una placa.

Objetivo final: la ESP32-S3-Touch-AMOLED-1.75C de Ángel usa la voz nativa de Grok Bot a través del puente del Mac (`src/`), en español y con la hora de `Europe/Madrid`.

## Fuentes consultadas

| Fuente | Versión | Qué aporta |
|---|---|---|
| Este repo, `firmware/pet/components/board_port/vendor/board_waveshare_s3_143c.c` | `fcbd820` | Placa actual (1.43C) |
| [Liu-zhongxian/zongzhu-bot](https://github.com/Liu-zhongxian/zongzhu-bot), `xiaozhi-esp32/main/boards/waveshare/esp32-s3-touch-amoled-1.75/` (`config.h`, `esp32-s3-touch-amoled-1.75.cc`, `config.json`) | `62ebe83` (MIT) | Pines, init del panel, PMU y táctil de la 1.75C en firmware que funciona |
| [waveshareteam/ESP32-S3-Touch-AMOLED-1.75C](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75C) (`pin_config.h`, `examples/esp-idf/02_lvgl_demo_v9`) y [docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.75C](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.75C) | `4bfdc80` (Apache-2.0) | Confirmación de pines, 32 MB de flash + 8 MB de PSRAM octal, firmware de fábrica |

La antigua URL de la wiki (`waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.75C`) devuelve 404; la documentación se ha movido a `docs.waveshare.com`. Los pines de zongzhu-bot y los del `pin_config.h` oficial coinciden en todo lo que ambos definen.

Datos que **ninguna fuente deja claros** y hay que sacar del esquemático (`hardware/schematic/ESP32-S3-Touch-AMOLED-1.75C-schematic.pdf` del repo oficial) antes de escribir el driver: qué canales del ES7210 son micrófono y cuál es la referencia del eco, a qué pin del AXP2101 va el botón PWR, qué raíles (DCDC/ALDO/BLDO) alimentan pantalla, táctil, códecs y micrófonos, si hay pin TE de la pantalla, y el modelo y dirección del RTC.

---

## 1. Diferencias de hardware

### 1.1 Resumen

| | 1.43C (actual) | 1.75C (destino) |
|---|---|---|
| SoC | ESP32-S3R8 | ESP32-S3R8 |
| Flash | 8 MB | **32 MB** (QIO en los ejemplos oficiales) |
| PSRAM | 8 MB octal | 8 MB octal (igual) |
| Pantalla | 466×466 AMOLED, controlador compatible CO5300/SH8601 | 466×466 AMOLED, **CO5300** |
| Táctil | CST820 en 0x15 | **CST9217 en 0x5A** |
| Gestión de energía | Sin PMU. LDO RT9193 por GPIO, cargador ETA6098, batería por ADC | **AXP2101 en 0x34** (raíles, carga, medidor de batería, botón PWR) |
| Audio | ES8311 (salida) + ES7210 (micros), I2S0 | Igual, otros pines. Con canal de referencia para cancelar el eco (`AUDIO_INPUT_REFERENCE true` en xiaozhi) |
| IMU | — | QMI8658 |
| RTC | — | Sí (modelo sin confirmar, probablemente PCF85063) |
| Expansor de E/S | — | No (el TCA9554 es solo de la 1.75 sin «C») |
| Botones | BOOT (GPIO0) + interruptor físico PWR | BOOT (GPIO0) + **PWR por PMU** (apagar con pulsación larga) |

### 1.2 Pines

| Función | 1.43C | 1.75C | Comentario |
|---|---|---|---|
| QSPI CS | 15 | **12** | |
| QSPI SCLK | 14 | **38** | |
| QSPI D0–D3 | 9, 10, 11, 12 | **4, 5, 6, 7** | |
| LCD RST | 13 | **1** | |
| LCD TE | 8 | **no definido** | En la 1.75C, GPIO8 es la salida I2S. El `init()` actual lo configura como entrada con pull-up: **no puede reutilizarse tal cual** |
| I2C SDA / SCL | 47 / 48 | **15 / 14** | Bus compartido: códecs, táctil, AXP2101, QMI8658, RTC |
| Táctil RST / INT | 16 / 17 | **2 / 11** | |
| I2S MCLK | 38 | **16** | |
| I2S BCLK | 39 | **9** | |
| I2S WS (LRCK) | 40 | **45** | GPIO45 es pin de arranque (tensión de VDD_SPI); como salida después del arranque no hay problema, pero no debe tener pull-up externo durante el reset |
| I2S DOUT (ESP → ES8311) | 41 | **8** | |
| I2S DIN (ES7210 → ESP) | 42 | **10** | |
| Amplificador (PA_EN) | 46 | 46 | Igual. También es pin de arranque, como ya pasaba en la 1.43C |
| Alimentación de códecs | GPIO18 (RT9193) | **AXP2101** (ALDO1 según xiaozhi: «MIC») | Por confirmar en el esquemático |
| Batería | ADC1 canal 3 (GPIO4) + estado de carga en GPIO7 | **AXP2101** (medidor y estado por I2C) | En la 1.75C, GPIO4 y GPIO7 son datos de la pantalla: el código del ADC **tiene que desaparecer** |
| BOOT | 0 | 0 | Igual |
| PWR | interruptor físico | PWRON del AXP2101 | xiaozhi configura 4 s de pulsación para apagar |

Casi ningún pin coincide, y varios de la 1.43C son señales activas en la 1.75C (8, 9, 10, 11, 12, 14, 15, 38, 4, 7). Si se arranca por error el binario de la 1.43C en la 1.75C, la pantalla no funcionará y se pondrán pulls y salidas sobre líneas que no son suyas. El instalador tiene que impedirlo (ver §2).

### 1.3 Pantalla (CO5300)

Mismo componente `espressif/esp_lcd_co5300` (2.2.0, ya en `idf_component.yml`) y mismo envío por bandas (`muse_lcd_bands.c`). Diferencias en la inicialización según xiaozhi:

- Antes de la tabla de siempre van tres órdenes para pasar a modo QSPI: `0xFE 0x20`, `0x19 0x10`, `0x1C 0xA0`, y después `0xFE 0x00`.
- **Sin** `0x36 0xC0` (MADCTL). En la 1.43C invertía los dos ejes y el táctil compensaba. En la 1.75C la orientación se corrige en el táctil (`mirror_x = mirror_y = 1`).
- Desfase de columna **6** (`esp_lcd_panel_set_gap(panel, 6, 0)`; 8 en la 1.43C). El ejemplo oficial de Arduino también usa 6.
- Esperas de 600 ms tras `0x2B` y `0x11` (100 ms en la 1.43C).
- El brillo se sigue enviando por `0x51` con el opcode `0x02`: el código actual sirve.
- El `round_area` (redondear a pares) se mantiene.

### 1.4 Táctil (CST9217)

Hay que **sustituir** `touch_read()`: el CST820 se lee a mano en el registro 0x02 y el CST9217 tiene otro protocolo. Dos opciones:

1. **Recomendada:** el componente `waveshare/esp_lcd_touch_cst9217` (lo usa xiaozhi, `^1.0.3`) más `esp_lcd_touch`, y leer desde nuestro `touch_read` de LVGL con `esp_lcd_touch_read_data()` + `esp_lcd_touch_get_coordinates()`. Se mantiene la regla actual de que solo LVGL toca el táctil.
2. Lectura I2C propia, como con el CST820. Menos dependencias, pero se reimplementa un protocolo sin documentación.

Reset: pulso por GPIO2 y nivel activo bajo, según xiaozhi. Espejo en X e Y.

### 1.5 PMU (AXP2101), nuevo

No existe en la 1.43C. Hay que **añadir** un pequeño driver I2C (registros, sin librería pesada) que:

- **Solo lectura** (D9): no se escribe ningún registro de raíles ni de carga en la primera versión.
- **Lea**: porcentaje de batería (0xA4), estado de carga y presencia de VBUS (0x00/0x01), tensión de batería. Sustituye a `read_power()` y su ADC.
- **Encienda** el raíl de los micrófonos/códecs antes de abrirlos (`codec_power`), si el esquemático confirma qué raíl es.
- `power_off` (apagado por software, Â«ApagarÂ» en Ajustes) y el apagado con PWR quedan para la fase 8 (D8).
- Más adelante: leer la IRQ de pulsación corta de PWRON para `MUSE_BTN_POWER_PRESS`.

**Cuidado:** el constructor `Pmic` de xiaozhi apaga todos los DCDC salvo DC1 y todos los LDO salvo ALDO1, y cambia la tensión de carga (4,1 V) y la corriente (200 mA). Copiarlo a ciegas puede dejar sin alimentación algo que la placa necesita o cambiar la carga de la batería. Plan: la primera versión **solo lee** y enciende el raíl confirmado. Cualquier escritura de configuración se justifica con el esquemático y el ejemplo oficial `01_AXP2101`.

### 1.6 Audio

- Mismo par ES8311 + ES7210 y mismas direcciones por defecto (`ES8311_CODEC_DEFAULT_ADDR`, `ES7210_CODEC_DEFAULT_ADDR`; xiaozhi no las cambia). Se mantiene la comprobación de `0x18` y `0x40`.
- Solo cambian los pines (§1.2) y la alimentación (PMU en lugar de GPIO18).
- `mic_selected` y `mic_slot`: en la 1.43C, MIC1+MIC2 son los micrófonos y 3/4 la referencia del amplificador. En la 1.75C, xiaozhi abre el ES7210 con referencia de entrada (TDM de 4 canales en su `BoxAudioCodec`). Nuestro firmware usa I2S estándar de 2 ranuras sin cancelación de eco en el dispositivo. Hay que confirmar en el esquemático qué canal es cada uno; mientras tanto, empezar con MIC1+MIC2, `mic_slot = 0`, y medir.
- Frecuencia: el firmware trabaja a 16 kHz (`MUSE_AUDIO_RATE`) y el puente remuestrea a 24 kHz. No cambia.

### 1.7 IMU y RTC (opcionales)

No son necesarios para el objetivo. El RTC sí es interesante: guardaría la hora con el Mac desconectado (hoy la hora solo llega del puente). Van al final del plan.

### 1.8 Qué se toca en `firmware/`

| Archivo | Cambio |
|---|---|
| `components/board_port/vendor/board_waveshare_s3_175c.c` | **Nuevo**: pines, init del panel, CST9217, AXP2101, audio, `read_power`, `power_off` |
| `components/board_port/vendor/axp2101.{c,h}` | **Nuevo**: driver mínimo por registros |
| `components/board_port/CMakeLists.txt` + `Kconfig` nuevo | Elegir la placa (`CONFIG_PET_BOARD_WAVESHARE_143C` / `_175C`) y compilar solo su archivo |
| `main/idf_component.yml` | Añadir `waveshare/esp_lcd_touch_cst9217` (y `esp_lcd_touch`), con versión fija como el resto |
| `sdkconfig.defaults` → `sdkconfig.defaults` común + `sdkconfig.defaults.143c` / `.175c` | `FLASHSIZE_32MB`, archivo de particiones y placa por variante |
| `partitions.csv` → `partitions_143c.csv` + `partitions_175c.csv` | Ver §2 |
| `main/pet_app.c` | El JSON de estado usa `pet_board()->name` en vez de la cadena fija de la 1.43C |
| `components/pet_provision/pet_provision.c` | Nombre BLE según la placa (hoy es fijo: `"Grok Pet 1.43C"`) |
| `components/board_port/vendor/provenance.json` | Registrar el origen de lo que se adapte de zongzhu-bot (MIT) y de Waveshare (Apache-2.0); actualizar `THIRD_PARTY_NOTICES.md` |
| `tools/build_pet.py`, `tools/pet_artifacts.py`, `tools/pet_install.py`, `tools/vendor/ports.py` | Perfil por placa (ver §2) |

El resto (`pet_core`, `pet_link`, `pet_audio`, `pet_network`, `pet_store`, `pet_ui`) es independiente de la placa, salvo los textos (§3).

---

## 2. Tabla de particiones de 32 MB e instalador

### 2.1 Propuesta

Mantiene el mismo comienzo que la tabla Muse actual (desplazamientos de `nvs`, `otadata`, `phy_init` y `ota_0`). Así el instalador puede seguir comprobando el arranque con la misma lógica y el bootloader conserva `CONFIG_PARTITION_TABLE_OFFSET=0x10000`.

```csv
# Name,    Type, SubType,  Offset,    Size
nvs,       data, nvs,      0x11000,   0xC000,
otadata,   data, ota,      0x1D000,   0x2000,
phy_init,  data, phy,      0x1F000,   0x1000,
ota_0,     app,  ota_0,    0x20000,   0x7E0000,
ota_1,     app,  ota_1,    0x800000,  0x7E0000,
coredump,  data, coredump, 0xFE0000,  0x20000,
storage,   data, littlefs, 0x1000000, 0xFFE000,
prod_data, data, 0x40,     0x1FFE000, 0x1000,
prod_bak,  data, 0x41,     0x1FFF000, 0x1000,
```

- Dos ranuras de app de casi 8 MB (hoy la app ocupa 2,2 MB: `0x22E8E0`, comprobado con el build de `fcbd820`). Hay margen para fuentes y animaciones.
- `coredump` (128 KB) para diagnosticar fallos en la placa nueva sin depurador.
- `storage` (~16 MB) libre para el futuro (sonidos, assets); el firmware no lo monta hasta que haga falta.
- `prod_data`/`prod_bak` se conservan al final, como en Muse. El firmware actual no las usa (decisiÃ³n D6).

### 2.2 Qué pasa con el instalador guardado

`firmware/tools/pet_install.py` está pensado para **un único caso**: 1.43C, 8 MB, tabla Muse ya instalada y `ota_0` confirmado. Solo escribe la ranura de la app. Con la 1.75C fallaría, a propósito, en varios puntos:

| Punto | Hoy | Resultado en la 1.75C |
|---|---|---|
| `--board` / `BOARD` | Solo la 1.43C | `BOARD_CONFIRMATION_REQUIRED` |
| `FLASH_BYTES = 0x800000` + `flash_id` | 8 MB | `WRONG_FLASH_SIZE` (la 1.75C da 32 MB) |
| Copia de seguridad | `read_flash(0, 0x800000)` | Copiaría solo la cuarta parte de la flash |
| `require_safe_layout` | Tabla Muse de 8 MB exacta | `PARTITION_LAYOUT_UNSUPPORTED` (la placa viene con el firmware de fábrica de Waveshare y otra tabla) |
| Instalación | Solo `write_flash(0x20000, app)` | No hay forma de escribir el bootloader ni la tabla nueva la primera vez |
| `pet_artifacts.py` | `BOARD`, offset `0x20000`, tamaño máximo `0x3E0000`, copia de `0x800000` | Rechaza el artefacto y la recuperación |

Cambios propuestos (en una PR propia, con tests simulados y **sin ejecutar nunca contra la placa**):

1. **Perfiles por placa** en un único sitio (`firmware/tools/pet_boards.py`): nombre, tamaño de flash, tabla esperada, rango de la app y nombre del binario. `build_pet.py`, `pet_artifacts.py`, `pet_install.py` y el manifiesto toman la placa de ahí. El manifiesto del artefacto incluye la placa y `require_board` cruza artefacto, copia y placa confirmada.
2. **Copia completa del tamaño real** (32 MB), con la misma disciplina que hoy: archivo `.partial`, `verify_flash`, SHA-256, `.local/` con permisos 600 y diario de recuperación.
3. **Nueva acción `migrate`** (primera instalación, cambio de tabla). Solo si hay copia completa verificada de *ese* dispositivo (hash del número de serie USB). Hace falta además un flag explícito (`--i-have-a-verified-backup`). Escribe bootloader + tabla + `otadata` limpio + app con `write_flash` y `verify_flash`, y relee la tabla al terminar. Es la única acción que toca el bootloader y la tabla.
4. **Nueva acción `restore-full`**: vuelve a escribir la copia completa (los 32 MB) en la placa de la que salió. Es la vuelta atrás de `migrate`.
5. `install` (solo app) sigue igual que ahora para las actualizaciones, pero contra la tabla de 32 MB de §2.1.
6. `tests/pet_install_test.py`: casos de 32 MB (`flash_id` con `0x19`), tabla de fábrica rechazada por `install` y aceptada por `migrate`, `migrate` sin copia rechazado, copia de otro dispositivo rechazada, y el binario de la 1.43C rechazado en la 1.75C y al revés.

Esto cubre el punto de `docs/ROADMAP.md` «非默认 OTA 状态或其他布局的受验证迁移路径» (ruta de migración verificada para otros layouts).

---

## 3. Idioma y zona horaria

### 3.1 Textos del firmware (pantalla)

Cadenas en chino que se ven en la placa, por archivo:

| Archivo | Líneas | Ejemplos |
|---|---|---|
| `components/pet_ui/pet_ui.c` | 19 | «嗯，我在听» (te escucho), «按住 BOOT 说话» (mantén BOOT para hablar), menú de ajustes (volumen, brillo, compañía silenciosa, subtítulos, altavoz, conexión) |
| `main/pet_app.c` | 7 | Estado del Mac, de la voz y del Wi-Fi |
| `components/pet_network/pet_network.c` | 5 | Estados del Wi-Fi |
| `components/pet_provision/pet_provision.c` | 4 | Configuración de red por BLE |

Plan:

- Sacar todas las cadenas a `components/pet_ui/pet_strings.h` (una tabla `PET_STR_*`), con `pet_strings_es.c` y, si se quiere conservar, `pet_strings_zh.c`. Se elige con `CONFIG_PET_LANG_ES` / `_ZH` (decisiÃ³n D3: espaÃ±ol por defecto, y la fuente CJK solo con `_ZH`).
- **Fuente:** la Montserrat que trae LVGL solo cubre ASCII (0x20–0x7E). Faltan `á é í ó ú ü ñ Ñ ¿ ¡` y saldrían como cuadros. Hay que generar con `lv_font_conv` una Montserrat 16 (OFL, la misma familia y tamaño que ya se usan, así no cambia la tipografía) con los rangos `0x20-0x7E,0xA0-0xFF,0x2026,0x2019,0x201C,0x201D`. La fuente Unifont CJK (`muse_font_cjk_16.c`, ~155.000 líneas) queda solo para la variante en chino.
- **Maquetación:** las etiquetas de `pet_ui.c` van en posiciones absolutas pensadas para el ancho de los ideogramas (p. ej. `label(root,…,260)` en 103,391). En español los textos son 2–3 veces más largos («Conexión y configuración de red»). Hay que revisar cada etiqueta dentro del círculo de 466 px: textos cortos, `LV_LABEL_LONG_WRAP` y centrado. Comprobación con `pet.snapshot` por USB (ya existe en `pet_console.c`) una vez instalada.
- Nombre en pantalla: hoy, sin batería se muestra «小糖» (el nombre del bot). Pasa a ser `CONFIG_PET_DISPLAY_NAME`, Â«Grok OrbÂ» por defecto (decisiÃ³n D2).

### 3.2 Textos y voz del puente (`src/`)

| Dónde | Hoy | Cambio |
|---|---|---|
| `src/native-bot.mjs:28` | `spokenLanguage: 'Chinese'` | Configurable; por defecto `'Spanish'` |
| `src/realtime.mjs:9` | `language = 'zh'` (`language_hint` de `grok-transcribe`) | `'es'`, y pasarlo desde `native-bot.mjs` (hoy no se pasa y se usa el valor por defecto) |
| `src/backend.mjs:45` | `'x-cursor-timezone': 'Asia/Shanghai'` | Zona configurada |
| `src/cli.mjs` | Bot por defecto `小糖`, mensajes en chino | Bot por defecto configurable, mensajes en español |
| `src/control.mjs`, `src/setup-page.mjs` | `lang="zh-CN"` y textos de los paneles | `lang="es"` y textos en español |
| `src/live-acceptance.mjs`, `hardware-interrupt.mjs`, `device-task-acceptance.mjs`, `pet-silent-task-acceptance.mjs`, `firmware/tools/hardware_acceptance.py` | Frases de prueba en chino | Frases en español (las de reconocimiento de voz tienen que tener sentido en español) |
| `native/Info.plist` | `NSMicrophoneUsageDescription` en chino | En español |
| `*.command` | Lanzadores con nombre en chino | Renombrar en la fase 9 (D7) |

Las instrucciones («prompts») de la voz **no están en este repo**: llegan del servicio (`VoiceCallHarnessSession`) según `spokenLanguage`. No hay plantilla local que traducir. Riesgo: no sabemos si el servicio acepta `'Spanish'` con ese valor exacto. Se comprueba en la fase 1 con la prueba de voz solo en el Mac (`npm start -- serve`), sin placa.

Configuración: un único sitio (`src/device-config.mjs` o variables de entorno) con `language` (`es`), `spokenLanguage` (`Spanish`), `timeZone` (`Europe/Madrid`) y `bot`. Se valida al arrancar (zona con `Intl.DateTimeFormat`) y queda cubierto con `npm test`.

### 3.3 Zona horaria

Hoy hay dos sitios fijados a Shanghái:

1. `src/backend.mjs:45`: cabecera `x-cursor-timezone` (afecta a lo que el servicio cree que es «ahora»).
2. `firmware/pet/components/pet_link/pet_link.c:83`: suma `8*3600` al `epochMs` que manda el puente cada 30 s (`src/device-server.mjs:108`). Con eso, `pet_core.c:140` decide la ventana de saludos (8:00–22:00) y el contador diario.

Propuesta (recomendada): **el Mac calcula el desfase y el firmware solo lo aplica.**

- El mensaje `clock` pasa a `{op:'clock', epochMs, utcOffsetMin}`. El puente calcula `utcOffsetMin` con `Intl` para la zona configurada en ese instante. Como se reenvía cada 30 s, el cambio de hora de marzo y octubre se aplica solo, en menos de 30 s.
- `src/pet-protocol.mjs` valida `utcOffsetMin` como entero en [-720, 840]. El firmware lo acepta en ese rango; si falta, usa 0 (UTC) en lugar de +8. No hace falta guardar reglas de zona horaria en la placa.
- Tests: `tests/pet-protocol.test.mjs` y el harness C `tests/pet_link_harness.c` (lo compila `tests/pet-link.test.mjs` en el host), con casos a ambos lados del cambio de hora de Madrid (último domingo de marzo y de octubre).

Alternativa: guardar en NVS una cadena POSIX (`CET-1CEST,M3.5.0,M10.5.0/3`) y usar `setenv("TZ")` + `localtime_r` en la placa. Así hay hora local sin Mac si además se usa el RTC, pero cada zona exige su cadena POSIX y cambiarla implica reconfigurar la placa. Tiene sentido solo cuando se añada el RTC (fase 8).

---

## 4. Plan por fases

Cada fase es una PR pequeña contra `main`. Las fases 1 a 6 se comprueban **sin placa**: `npm test`, `python3 scripts/check-publication.py` y build del firmware para `esp32s3` con ESP-IDF 6.0.1 (`~/esp/esp-idf-v6.0.1`). La fase 7 es la única que toca hardware y la hace Ángel, a mano y siguiendo §5.

| Fase | PR | Contenido | Cómo se comprueba |
|---|---|---|---|
| 0 | Este documento | Plan | `npm test` (79 pasan, 3 omitidos: son las comprobaciones de dependencias del target, que se saltan sin build); el build actual de 1.43C compila (app `0x22E8E0`, 44 % libre) |
| 1 | Puente en español + zona horaria | §3.2 y la parte del puente de §3.3 (`utcOffsetMin`, cabecera, `spokenLanguage`, `language_hint`, textos de los paneles y de la CLI, bot «Grok Orb» por defecto). Las frases de las pruebas de aceptación (`live-acceptance.mjs`, `hardware-interrupt.mjs`, `*-task-acceptance.mjs`, `hardware_acceptance.py`) necesitan una voz de síntesis en español y se traducen al preparar la fase 7 | `npm test` con tests nuevos de configuración, zona y cambio de hora. Prueba manual de voz **solo en el Mac** (`npm start -- serve`) para confirmar que el servicio acepta `Spanish` |
| 2 | Reloj del firmware | `pet_link.c` usa `utcOffsetMin`; se quita el `+8*3600` | `npm test` (harness C de `pet_link`); build 1.43C |
| 3 | Selección de placa | Kconfig de placa, `sdkconfig.defaults.<placa>`, particiones por placa, `build_pet.py --board`, nombre de placa en estado/BLE desde `pet_board()->name`. **Sin** código de la 1.75C todavía | Build 1.43C con el mismo `sdkconfig` efectivo que antes (comparar hash) y la misma app |
| 4 | Placa 1.75C | `board_waveshare_s3_175c.c`, `axp2101.c` en modo lectura + raíl de los códecs, CST9217, tabla de 32 MB (§2.1), procedencia y licencias | Build `--board 175c` correcto, app ≤ `0x7E0000`; test de host que detecta GPIOs repetidos o fuera de rango en las placas; build 1.43C sin cambios |
| 5 | Interfaz en español | Tabla de cadenas, Montserrat 16 con Latin-1, revisión de la maquetación | Build de las dos placas; test de host que comprueba que todos los caracteres de `pet_strings_es.c` están en la fuente y que ninguna cadena supera su ancho máximo |
| 6 | Instalador multiplaca | §2.2: perfiles, copia de 32 MB, `migrate`, `restore-full` | `npm test` + `tests/pet_install_test.py` con esptool simulado. Nunca contra la placa |
| 7 | Primera instalación (manual, Ángel) | Copia completa → `migrate` → configuración de red por USB → `npm start` con la placa | Lista de `docs/VALIDATION.md` adaptada: pantalla, táctil, BOOT, voz en español, hora de Madrid, batería, eco |
| 8 | Extras | Apagado con PWR (4 s, D8), `power_off`, RTC y zona en la placa para tener hora sin Mac, QMI8658 | Build y prueba en la placa |
| 9 | Documentación en español (D7) | `README.md`, `GETTING_STARTED`, `VALIDATION`, renombrar los `.command` | `npm test`, `check-publication.py` |

Las fases 1–2 y 3–6 son independientes y pueden avanzar en paralelo.

**Solapamiento con la PR #1.** La rama `claude/port-175c-es` (PR #1, abierta antes que este plan) ya hace en un solo bloque buena parte de las fases 1 a 6: placa 1.75C, particiones, instalador, fuente latina y textos en español, además de su propio `docs/PORT_1.75C.md`. Antes de seguir con más fases hay que decidir cuál de las dos ramas manda, para que no choquen al fusionar.

**Pruebas de aceptación.** Mientras sus frases sigan en chino, `live-acceptance.mjs` y `*-task-acceptance.mjs` fijan la voz a `zh`. Para `hardware-interrupt.mjs`, que usa un puente ya arrancado, hay que lanzar ese puente con `--language zh`.

---

## 5. Riesgos y copia de seguridad

### 5.1 Riesgos

| Riesgo | Consecuencia | Mitigación |
|---|---|---|
| Configuración equivocada del AXP2101 | Algo se queda sin alimentación, la batería se carga mal o se apaga solo | Primera versión solo de lectura; escrituras justificadas con esquemático y ejemplo oficial; nunca tocar tensiones de carga sin revisarlo |
| Binario de una placa arrancado en la otra | Pines en conflicto (§1.2) y pantalla negra | El instalador cruza placa, artefacto y tamaño de flash; nombre de placa en el manifiesto |
| Pines de arranque (0, 3, 45, 46) | No arranca o entra en modo descarga | No usar 3; 45 y 46 solo como salidas después del arranque, como en xiaozhi |
| eFuses (cifrado de flash o arranque seguro) activos de fábrica | Una copia «en claro» no serviría y escribir podría dejar la placa inservible | Leer `espefuse summary` (solo lectura) antes de nada; si hay cifrado o arranque seguro, **parar**. Nunca quemar eFuses |
| `migrate` interrumpido a mitad | Bootloader o tabla a medias | El ROM del ESP32-S3 siempre permite el modo descarga (mantener BOOT y reconectar), así que siempre se puede volver a escribir la copia completa; además, el firmware de fábrica de Waveshare (`ESP32-S3-Touch-AMOLED-1.75C-FactoryOnly-260114.bin`, release v1.0.1) como segunda vía |
| Canales del ES7210 o eco | Micrófono mudo, o el bot se oye a sí mismo y se interrumpe | Confirmar en el esquemático; medir con `firmware/tools/fixture_audio.py` y las pruebas de interrupción |
| El servicio no acepta `spokenLanguage: 'Spanish'` | Voz en otro idioma o error del servicio | Fase 1 probada solo en el Mac antes de tocar el firmware |
| API privada de Grok Bot | Puede cambiar sin aviso | Fuera de nuestro control; tests de contrato existentes en `tests/backend.test.mjs` |
| Licencias | Copiar código MIT o Apache sin aviso | Registrar en `provenance.json` y `THIRD_PARTY_NOTICES.md` cada fragmento adaptado |

### 5.2 Copia completa de la flash (antes de cualquier instalación)

**Lo hace Ángel, a mano.** Ni Claude ni ninguna automatización de este repo lo ejecuta. Son operaciones de solo lectura salvo la última sección. Con el entorno de ESP-IDF 6.0.1 (esptool v5):

```sh
# 0. Entorno y puerto (solo una placa conectada)
. ~/esp/esp-idf-v6.0.1/export.sh
ls /dev/cu.usbmodem*                       # anota el puerto: PORT=/dev/cu.usbmodemXXXX
mkdir -p ~/grok-pet-backups && chmod 700 ~/grok-pet-backups && cd ~/grok-pet-backups

# 1. Identificar chip y flash (debe decir ESP32-S3 y 32MB)
esptool --chip esp32s3 --port "$PORT" flash-id | tee 175c-flash-id.txt

# 2. Estado de seguridad (solo lectura). Si hay cifrado de flash o arranque seguro activos: PARAR
espefuse --chip esp32s3 --port "$PORT" summary | tee 175c-efuse-summary.txt

# 3. Copia completa, dos veces
esptool --chip esp32s3 --port "$PORT" --baud 921600 read-flash 0 ALL 175c-full-a.bin
esptool --chip esp32s3 --port "$PORT" --baud 921600 read-flash 0 ALL 175c-full-b.bin

# 4. Comprobar: las dos copias son idénticas, miden 33554432 bytes y coinciden con la placa
ls -l 175c-full-*.bin
shasum -a 256 175c-full-a.bin 175c-full-b.bin | tee 175c-full.sha256
esptool --chip esp32s3 --port "$PORT" verify-flash 0 175c-full-a.bin

# 5. Tabla de particiones de fábrica, para documentarla (archivo local, no toca la placa).
#    Lo habitual es 0x8000 (skip=8); si sale vacía, probar 0x10000 (skip=16).
dd if=175c-full-a.bin of=175c-factory-table.bin bs=4096 skip=8 count=1
python ~/esp/esp-idf-v6.0.1/components/partition_table/gen_esp32part.py \
  175c-factory-table.bin | tee 175c-factory-partitions.txt
```

- Guardar `175c-full-a.bin`, el `.sha256` y los `.txt` **fuera del repo** y en un segundo sitio (disco externo o similar). La copia contiene la NVS de fábrica (posibles credenciales de Wi-Fi), así que es privada.
- Descargar también el firmware de fábrica oficial de la release v1.0.1 de `waveshareteam/ESP32-S3-Touch-AMOLED-1.75C` como segunda vía de recuperación.
- Si la placa no responde por USB: mantener BOOT, desconectar y volver a conectar (o pulsar PWR) para entrar en modo descarga. Según la FAQ de Waveshare, al terminar hay que apagarla y encenderla a mano.

Restaurar (solo si hace falta, **escribe en la placa**):

```sh
esptool --chip esp32s3 --port "$PORT" --baud 921600 write-flash 0 175c-full-a.bin
esptool --chip esp32s3 --port "$PORT" verify-flash 0 175c-full-a.bin
```

Cuando exista la fase 6, `pet_install.py backup --board "Waveshare ESP32-S3-Touch-AMOLED-1.75C"` hará lo mismo con registro y verificación. La copia manual de arriba sigue siendo recomendable como copia independiente.

---

## 6. Decisiones

Tomadas el 2026-11-06 en nombre de Ángel. Sustituyen a las recomendaciones de las secciones anteriores donde no coincidan.

| # | Pregunta | Decisión | Consecuencia en el plan |
|---|---|---|---|
| D1 | ¿Soporte para la 1.43C? | Se mantienen las dos placas | Fase 3 (selección de placa) obligatoria; cada PR de firmware compila las dos |
| D2 | Nombre del bot | Bot **«Grok Orb»** y nombre en pantalla «Grok Orb», configurables | Puente: `--bot` (y `GROK_BOT_NAME`) con «Grok Orb» por defecto (fase 1). Firmware: `CONFIG_PET_DISPLAY_NAME` con «Grok Orb» por defecto (fase 5) |
| D3 | Idioma de la interfaz | Elegible por Kconfig, español por defecto, **sin la fuente CJK en el build en español** | Fase 5: `CONFIG_PET_LANG_ES` / `_ZH`; `muse_font_cjk_16.c` solo se compila con `_ZH` |
| D4 | Hora | Desfase desde el Mac ahora; RTC y zona en la placa en la fase 8 | Fases 1 y 2 según §3.3 |
| D5 | Firmware actual de la placa | **Desconocido.** Antes de cualquier flasheo se hace copia completa y se lee la tabla de particiones de esa copia; no se asume ninguna | El instalador (fase 6) no supone ninguna tabla de origen: `migrate` solo exige una copia completa verificada del mismo dispositivo, y la tabla de fábrica se documenta a partir de esa copia (§5.2, paso 5) |
| D6 | `prod_data`/`prod_bak` | Se conservan | Tabla de §2.1 sin cambios |
| D7 | Traducir documentación y `.command` | Sí, en una PR aparte al final | Nueva fase 9 |
| D8 | Botón PWR | Solo apagar (4 s) en la primera versión | Fase 8: sin acción con pulsación corta |
| D9 | Batería | Se asume alimentación por USB; el AXP2101 en **solo lectura** y funcionando con o sin batería | Fase 4: `read_power` devuelve `battery_pct = -1` si no hay batería (bit de presencia del AXP2101) y no se escriben registros de carga ni de raíles. Si los códecs necesitan un raíl que llega apagado, se documenta y se decide antes de escribir nada |

Ninguna fase de este plan flashea la placa ni ejecuta instaladores sobre ella. La fase 7 la hace Ángel a mano.
