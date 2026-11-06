# Port a la Waveshare ESP32-S3-Touch-AMOLED-1.75C

Este documento recoge el plan del port de Grok Orb (fork de
[vliuyt/grok-bot-esp32-bridge](https://github.com/vliuyt/grok-bot-esp32-bridge))
a la Waveshare ESP32-S3-Touch-AMOLED-1.75C, las diferencias reales con la
1.43C para la que se escribió el proyecto, y cómo hacer la copia de seguridad
antes del primer flasheo.

> Estado: compila con ESP-IDF 6.0.1 y pasa los tests del Mac, pero **no se ha
> probado en una placa real**. Lo que depende del hardware está en
> [Pendiente de comprobar en la placa](#pendiente-de-comprobar-en-la-placa).

## Fuentes

Todo lo de abajo se ha contrastado con estas fuentes (no con la memoria):

| Fuente | Qué se ha sacado |
|---|---|
| BSP oficial `waveshare/esp32_s3_touch_amoled_1_75c` 3.0.0 (ESP Component Registry, Apache-2.0) | Pines, tabla de inicio del CO5300, `gap` de 6 px, espejo del táctil, ganancia del ES8311 |
| Driver `waveshare/esp_lcd_touch_cst9217` 2.0.0 (Apache-2.0) | Dirección 0x5A, registro 0xD000, informe de 10 bytes, byte de ACK 0xAB, empaquetado de coordenadas, reset |
| [waveshareteam/ESP32-S3-Touch-AMOLED-1.75C](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75C) @ `4bfdc80` | Esquema (`hardware/schematic`), ejemplo `01_AXP2101`, `sdkconfig.defaults` de los ejemplos, firmware de fábrica |
| [Liu-zhongxian/zongzhu-bot](https://github.com/Liu-zhongxian/zongzhu-bot) @ `62ebe83`, `xiaozhi-esp32/main/boards/waveshare/esp32-s3-touch-amoled-1.75/` | Rama `_1_75C` de `config.h` y configuración del AXP2101 (confirma los pines del BSP) |

El código de esas fuentes usa otras APIs (BSP, `esp_lcd_touch`, C++ de
xiaozhi); se ha usado como referencia y el board port está escrito contra la
interfaz `muse_board_t` del proyecto.

## Diferencias reales con la 1.43C

| | 1.43C (original) | 1.75C | Comprobado en |
|---|---|---|---|
| SoC | ESP32-S3, 8 MB PSRAM octal | ESP32-S3R8, 8 MB PSRAM octal (igual `CONFIG_SPIRAM_MODE_OCT`) | Ejemplos oficiales |
| Flash | 8 MiB | **32 MiB** en la placa actual | Firmware de fábrica: 33 488 896 bytes y cabecera del bootloader con código de tamaño `0x5_` (32 MB) |
| Tabla de particiones de fábrica | Muse, en `0x10000` | Otra completamente distinta, en `0x8000`: `nvsfactory`, `nvs`, `otadata`, `phy_init`, `factory` (9 MiB), `ota_0`, `ota_1`, `assets`, `storage` | Tabla leída del firmware de fábrica |
| Pantalla | CO5300/SH8601 QSPI, D0-3 = 9-12, CLK 14, CS 15, RST 13, TE 8, MADCTL `0xC0`, gap 8 | CO5300 QSPI 466×466, **D0-3 = 4-7, CLK 38, CS 12, RST 1**, sin TE, sin MADCTL (orientación por defecto), **gap 6**, y la tabla de inicio empieza con la página `FE 20` / `19 10` / `1C A0` | BSP |
| Táctil | CST820 en 0x15, RST 16, INT 17 | **CST9217 en 0x5A**, RST 2, INT 11; registro de 16 bits `0xD000`, 10 bytes, ACK `0xAB`; ejes X e Y invertidos respecto a la pantalla | BSP + driver CST9217 |
| I2C | SDA 47, SCL 48 | **SDA 15, SCL 14** (táctil, códecs, PMIC e IMU en el mismo bus) | BSP |
| Audio | ES8311 + ES7210, MCLK 38, BCLK 39, WS 40, DOUT 41, DIN 42, PA 46, LDO de códecs en GPIO 18 | ES8311 + ES7210, **MCLK 16, BCLK 9, WS 45, DOUT 8, DIN 10**, PA 46; sin GPIO de alimentación de códecs | BSP + xiaozhi |
| Micrófonos | MIC1 + MIC2 del ES7210 | MIC1 + MIC2; MIC3 (`ADC_MIC3_P/N`) es la referencia del altavoz | Esquema |
| Alimentación | ETA6098 (carga en GPIO 7) y divisor de batería en GPIO 4 | **PMIC AXP2101** en 0x34: ALDO1 alimenta los micrófonos; batería, carga y VBUS se leen por I2C | Ejemplo `01_AXP2101`, xiaozhi, esquema |
| Botón | BOOT (GPIO 0) | BOOT (GPIO 0); PWR va al AXP2101 | BSP |
| IMU | — | QMI8658 en el bus I2C | xiaozhi (no se usa en este port) |

Detalles que no cuadraban con lo que se sabía antes del port:

- **Flash.** Los ejemplos oficiales no se ponen de acuerdo: tres usan
  `CONFIG_ESPTOOLPY_FLASHSIZE_32MB` y uno (`04_Immersive_block`) `16MB`. El
  firmware de fábrica confirma 32 MiB. El port escribe en la cabecera **16 MB**
  y su tabla acaba en `0xC20000`, así que funciona en unidades de 16 y de
  32 MiB; el instalador acepta los dos tamaños y siempre copia la flash
  entera, mida lo que mida.
- **El AXP2101.** El BSP oficial no lo toca: depende de la configuración de
  arranque del PMIC. xiaozhi y el ejemplo `01_AXP2101` sí configuran DC1 y
  ALDO1 a 3,3 V y apagan el resto. El port solo pone **ALDO1 a 3,3 V y lo
  enciende** (lectura-modificación-escritura), y activa la medida de tensión
  de la batería. No apaga otras salidas ni cambia el cargador.
- **El CST9217** tiene un modo de comandos (`0xD101`) que el driver de
  Waveshare usa para leer la versión. El port no entra en ese modo: hace el
  reset y lee informes, que es lo único que necesita.

## Qué se ha cambiado

1. **Board port nuevo**, `firmware/pet/components/board_port/vendor/board_waveshare_s3_175c.c`:
   pantalla CO5300 (mismo envío por bandas que la 1.43C), táctil CST9217 leído
   por I2C desde LVGL, ES8311/ES7210 en I2S0 a 16 kHz, AXP2101 (ALDO1, batería,
   carga) y BOOT para hablar. Se elige en compilación con
   `CONFIG_PET_BOARD_WAVESHARE_175C` (Kconfig en `board_port`); la 1.43C sigue
   siendo la opción por defecto y su archivo no cambia.
2. **Configuración por placa**: `firmware/pet/sdkconfig.defaults.175c` se aplica
   encima de `sdkconfig.defaults` (flash de 16 MB, tabla
   `partitions_175c.csv`, placa 1.75C). `build_pet.py --board 175c` la usa.
3. **Particiones** (`firmware/pet/partitions_175c.csv`): mismo offset de tabla
   (`0x10000`) y mismos primeros sectores que la 1.43C (`nvs`, `otadata`,
   `phy_init`) y dos ranuras de 6 MiB:

   | Nombre | Tipo | Inicio | Tamaño |
   |---|---|---:|---:|
   | nvs | data/nvs | `0x11000` | `0xC000` |
   | otadata | data/ota | `0x1D000` | `0x2000` |
   | phy_init | data/phy | `0x1F000` | `0x1000` |
   | ota_0 | app/ota_0 | `0x20000` | `0x600000` |
   | ota_1 | app/ota_1 | `0x620000` | `0x600000` |

4. **Instalador** (`firmware/tools/pet_install.py --board 'Waveshare ESP32-S3-Touch-AMOLED-1.75C'`), ver abajo.
5. **Español**: voz (`spokenLanguage: 'Spanish'`, `language_hint: 'es'`), textos
   de la pantalla y de las páginas del Mac, y fuente con tildes, ñ, ¿ y ¡.
6. **Zona horaria configurable**, `Europe/Madrid` por defecto.

## Filosofía de seguridad del instalador

Se conserva la del proyecto original:

- **Primero, una copia completa y verificada de la flash.** `backup` lee la
  flash entera (32 MiB en la placa actual), la vuelve a verificar contra el
  chip en la misma sesión, guarda el SHA-256 y la liga a la identidad USB de
  esa placa. Nunca sobrescribe una copia existente. Sin esa copia, `install`
  y `restore` no hacen nada.
- **Se comprueba que es la misma placa y que no ha cambiado.** `install`
  exige la copia de esa misma placa, el mismo tamaño de flash y una imagen
  construida con el código actual (si cambias el código, hay que volver a
  compilar).

La diferencia está en que la 1.75C **no** trae una tabla compatible con Muse,
así que no se puede instalar solo la aplicación como en la 1.43C:

- **Primera instalación** (la placa todavía tiene el arranque de fábrica, el
  mismo que hay en la copia): se escriben los primeros 128 KiB (bootloader en
  `0x0`, tabla en `0x10000`, `nvs`/`phy_init` borrados para no heredar datos
  de fábrica, y `otadata` con un registro confirmado que elige `ota_0`) y la
  aplicación en `0x20000`. Todo se verifica después de escribir. El resto de
  la flash no se toca.
- **Siguientes instalaciones** (la placa ya tiene esta tabla y `ota_0`
  confirmado): solo se escribe la aplicación en `0x20000`.
- Cualquier otro estado (bootloader o tabla distintos de los de la copia y
  que tampoco son los de este proyecto) se rechaza con `BOOT_METADATA_CHANGED`.
- **`restore`** escribe la copia completa en `0x0` y la verifica: la placa
  vuelve exactamente a como estaba al hacer la copia, firmware de fábrica
  incluido.

Si una escritura se corta a medias, la placa sigue pudiendo entrar en modo
descarga (es la ROM del ESP32-S3, no se borra): mantén BOOT pulsado, pulsa y
suelta RESET (o conecta el USB con BOOT pulsado) y repite `restore`.

## Copia de seguridad con esptool antes del primer flasheo

El instalador hace la copia por ti (paso 2). Además, conviene tener una copia
hecha a mano con `esptool`, independiente de este proyecto. Ninguna de las dos
se sube a Git (`.local/` está ignorado); guárdalas también fuera del repo.

1. Prepara las herramientas USB (una vez):

   ```sh
   python3 -m venv .local/hardware-venv
   .local/hardware-venv/bin/python -m pip install -r requirements.txt
   ```

2. Copia gestionada por el instalador (32 MiB, verificada, ligada a la placa):

   ```sh
   .local/hardware-venv/bin/python firmware/tools/pet_install.py backup \
     --board 'Waveshare ESP32-S3-Touch-AMOLED-1.75C'
   ```

   Crea `.local/before-pet-full-175c.bin` y `.local/device-backup-175c.json`.
   Con más de una placa conectada, añade `--port /dev/cu.usbmodemXXXX`.

3. Copia manual con esptool (cierra antes cualquier monitor serie). Primero
   confirma el tamaño real de la flash:

   ```sh
   .local/hardware-venv/bin/python -m esptool --chip esp32s3 --port /dev/cu.usbmodemXXXX flash-id
   ```

   Debe decir `Detected flash size: 32MB` (o `16MB`). Lee la flash entera con
   ese tamaño (`0x2000000` para 32 MB, `0x1000000` para 16 MB) y compruébala
   contra la placa:

   ```sh
   .local/hardware-venv/bin/python -m esptool --chip esp32s3 --port /dev/cu.usbmodemXXXX \
     -b 460800 read-flash 0 0x2000000 .local/1.75C-fabrica-manual.bin
   .local/hardware-venv/bin/python -m esptool --chip esp32s3 --port /dev/cu.usbmodemXXXX \
     -b 460800 verify-flash 0 .local/1.75C-fabrica-manual.bin
   shasum -a 256 .local/1.75C-fabrica-manual.bin
   ```

   `verify-flash` tiene que acabar en `Verification successful`. Apunta el
   SHA-256.

4. Para volver a fábrica sin este proyecto:

   ```sh
   .local/hardware-venv/bin/python -m esptool --chip esp32s3 --port /dev/cu.usbmodemXXXX \
     -b 460800 write-flash 0 .local/1.75C-fabrica-manual.bin
   ```

   Waveshare publica además la imagen de fábrica completa
   (`firmware/ESP32-S3-Touch-AMOLED-1.75C_FactoryOnly_20260114.bin` en su
   repositorio, para escribir en `0x0`). Sirve como último recurso, pero no
   tiene tus datos: la copia propia es la buena.

## Compilar e instalar

```sh
# ESP-IDF 6.0.1 instalado en ~/esp/esp-idf-v6.0.1 (./install.sh esp32s3)
python3 firmware/tools/build_pet.py --idf ~/esp/esp-idf-v6.0.1 --board 175c
npm test

# Solo cuando quieras flashear (no lo hace ningún otro paso):
.local/hardware-venv/bin/python firmware/tools/pet_install.py install \
  --board 'Waveshare ESP32-S3-Touch-AMOLED-1.75C'
# Volver al estado de la copia:
.local/hardware-venv/bin/python firmware/tools/pet_install.py restore \
  --board 'Waveshare ESP32-S3-Touch-AMOLED-1.75C'
```

El build deja `docs/pet-firmware-manifest-175c.json` y las imágenes en
`firmware/build/grok-pet-175c-*.bin` (aplicación, bootloader y tabla), todo
ignorado por Git. `build_pet.py` sin `--board` sigue compilando la 1.43C.
No uses `idf.py flash`: escribiría toda la tabla sin las comprobaciones.

## Idioma y zona horaria

El puente del Mac usa por defecto español y `Europe/Madrid`. Se cambian con
variables de entorno:

```sh
GROK_TIMEZONE=Atlantic/Canary node src/cli.mjs device --bot "Nombre del Bot"
GROK_LANGUAGE=en node src/cli.mjs serve --bot "Nombre del Bot"   # es (defecto), en, zh
```

- La zona va en la cabecera `x-cursor-timezone` del Bot y en el mensaje
  `clock` hacia la placa, que ahora incluye `offsetMin` (desfase UTC en ese
  momento, con horario de verano). Se reenvía cada 30 s, así que el cambio de
  hora se aplica solo. Un puente antiguo sin `offsetMin` sigue con UTC+8.
- La pantalla usa Montserrat 16 (la misma de antes) y una conversión de
  Montserrat Medium con Latin-1 (á é í ó ú ü ñ ¿ ¡ ç…), comillas, guiones, «…»
  y € como respaldo; detrás sigue la fuente CJK de Unifont.

## Pendiente de comprobar en la placa

No se ha podido verificar sin hardware:

- Que la pantalla arranca con la tabla y el `gap` del BSP a través del envío
  por bandas del proyecto, y que la orientación es la correcta.
- Que el táctil responde leyendo los informes sin pasar por el modo de
  comandos, y que el espejo de ejes coincide con la pantalla.
- Que los micrófonos dan señal con solo ALDO1 encendido y que MIC1/MIC2 son los
  buenos; el volumen y la ganancia del altavoz.
- Las lecturas del AXP2101 (porcentaje, tensión y estado de carga).
- Leer 32 MiB con esptool 5.6.0 (direcciones por encima de 16 MiB) y el tiempo
  que tarda la restauración completa.
- Que el Bot acepta `spokenLanguage: 'Spanish'` y `language_hint: 'es'` (el
  contrato nativo solo se probó en chino).
- Cómo quedan los textos en español en la pantalla redonda (no hay forma de
  sacar capturas sin la placa: `pet.snapshot` funciona por USB).
