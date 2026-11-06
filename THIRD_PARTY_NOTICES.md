# Third-party notices

Original project code is licensed under Apache-2.0; third-party material retains its own applicable license. The root license does not relicense fonts or trademarks.

## Muse Gadget SDK — Apache-2.0

Upstream: <https://github.com/facebookincubator/muse-gadget-sdk>.
Copyright (c) Meta Platforms, Inc. and affiliates.

Retained files include the board support headers and LCD-band implementation in `firmware/pet/components/board_port/vendor/`, `firmware/pet/partitions.csv`, and the unmodified `firmware/tools/vendor/chat.py` / `ports.py` USB helpers. Original notices remain in source. Full license: [Muse-Apache-2.0.txt](docs/third-party/Muse-Apache-2.0.txt).

`board_waveshare_s3_143c.c` is the project's local 1.43C board adaptation against the Muse board interfaces; it was not an existing file in the referenced public SDK checkout. Its existing Meta notice is retained for adapted material. The board's pin mappings and behavior were validated on the physical Waveshare 1.43C. No complete Muse cloud SDK, token or App is bundled. Exact retained file digests and provenance are in `firmware/pet/components/board_port/vendor/provenance.json`; non-board USB provenance is in `firmware/tools/vendor/PROVENANCE.json`.

`board_waveshare_s3_175c.c` is this fork's local 1.75C adaptation of `board_waveshare_s3_143c.c` against the same Muse interfaces, so it keeps the same Meta notice. Its pins, CO5300 initialization, CST9217 report format and AXP2101 registers were cross-checked against Waveshare's Apache-2.0 BSP `waveshare/esp32_s3_touch_amoled_1_75c` 3.0.0 and driver `waveshare/esp_lcd_touch_cst9217` 2.0.0, the `01_AXP2101` example and schematic in [waveshareteam/ESP32-S3-Touch-AMOLED-1.75C](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75C), and the 1.75C board of [Liu-zhongxian/zongzhu-bot](https://github.com/Liu-zhongxian/zongzhu-bot); no code from them is copied. It has not yet been validated on a physical board; see [docs/PORT_1.75C.md](docs/PORT_1.75C.md).

## Montserrat — OFL-1.1 for the distributed Latin-1 font

`firmware/pet/components/pet_ui/fonts/pet_font_latin_16.c` is an LVGL C conversion (lv_font_conv 1.5.3, 16 px, 4 bpp) of the Latin-1 supplement and a few punctuation glyphs of Montserrat Medium, the same family as LVGL's built-in Montserrat. Copyright 2011 The Montserrat Project Authors (<https://github.com/JulietaUla/Montserrat>). Source TTF: LVGL v9.5.0 `scripts/built_in_font/Montserrat-Medium.ttf`, SHA-256 `421f26b23e2be6b98373d32acd3cb2897b154d4bf0a77d26534ce476e4cbed53`. Distributed under **SIL Open Font License 1.1**, separately from the Apache-licensed application; license text in [Montserrat-OFL-1.1.txt](docs/third-party/Montserrat-OFL-1.1.txt).

## GNU Unifont 16.0.04 — OFL-1.1 for the distributed font

`firmware/pet/components/pet_ui/fonts/muse_font_cjk_16.c` and `pet_font_pixel_16.c` (8-bit theme) are LVGL C conversions of unaltered subsets of GNU Unifont glyphs. Font copyright:

Copyright © 1998-2025 Roman Czyborra, Paul Hardy, Qianqian Fang, Andrew Miller, Johnnie Weaver, David Corbett, Nils Moskopp, Rebecca Bettencourt, Ho-Seok Ee, et al.

Upstream version: <https://unifoundry.com/pub/unifont/unifont-16.0.04/>.
Source font `unifont-16.0.04.otf` SHA-256: `0e3981ab552231b5a2a870f2b61741903a4bf25c23ef5aeb05fdced1b3c7af4d`.

The upstream font is dual licensed; this repository distributes its font conversion under **SIL Open Font License 1.1**, separately from the Apache-licensed application. The upstream copyright and license texts are preserved in [Unifont-OFL-1.1.txt](docs/third-party/Unifont-OFL-1.1.txt) and [Unifont-16.0.04-COPYING.txt](docs/third-party/Unifont-16.0.04-COPYING.txt). Font regeneration details are in the font directory README. The current source release does not distribute font-generation utilities from Unifont.

## Dependencies resolved when building

ESP-IDF and managed component sources are not vendored in this repository. `firmware/pet/main/idf_component.yml` and `dependencies.lock` pin their versions: ESP-IDF, LVGL, esp_codec_dev, esp_lvgl_adapter, esp_lcd_co5300, cJSON and mDNS, including their transitive dependencies. Each has its own licenses/notices in the downloaded package.

If distributing compiled firmware later, collect licenses/notices for the actual linked dependency graph and embedded assets. In particular this includes NimBLE NOTICE, FreeRTOS/lwIP/cJSON, Unifont, and LVGL's Montserrat plus embedded FontAwesome font data. The initial release deliberately contains source only and does not claim this list alone is a complete binary redistribution bundle.

## Names and appearance

Grok, Grok Bot and other product names belong to their respective owners. The procedural pink-circle pet drawing is application UI code; this project does not distribute official App assets or grant rights to third-party trademarks. No vendor affiliation or endorsement is implied.
