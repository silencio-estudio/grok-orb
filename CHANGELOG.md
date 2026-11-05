# Changelog

## Sin publicar — port a la 1.75C y español (Grok Orb)

- Board port para la Waveshare ESP32-S3-Touch-AMOLED-1.75C (CO5300, CST9217, ES8311/ES7210, AXP2101, BOOT), seleccionable con `build_pet.py --board 175c`; la 1.43C sigue igual y por defecto.
- Tabla `partitions_175c.csv` y perfil del instalador para 16/32 MiB: copia completa verificada, primera instalación del área de arranque solo sobre la placa copiada y sin cambios, actualizaciones solo de la aplicación y restauración completa.
- Voz, textos de la pantalla y páginas del Mac en español; fuente Montserrat con Latin-1.
- Zona horaria configurable (`GROK_TIMEZONE`, `Europe/Madrid` por defecto), también en el reloj de la placa (`offsetMin`).
- Sin probar en hardware: ver `docs/PORT_1.75C.md`.

## 0.1.1 — 2026-10-19

- Keep voice on the physical BOOT button: hold to record, release to submit, hold again to interrupt.
- Remove the experimental touch voice button after real-device feedback; touchscreen remains for petting and settings.
- Display connection/listening/interruption instructions independently of conversation captions.
- Accept CRC-validated, confirmed ota_0 boot records for application-only upgrades; never rewrite boot metadata. Reject erased OTA records, whose first-boot initialization would otherwise invalidate safe recovery checks.

- Reference-device update, sanitized final-screen capture and 82 host tests passed. The user verified voice on the earlier touch/BOOT build; the final BOOT-only UI build was checked silently.

## 0.1.0 — 2026-10-19

Initial public source release.

- Standalone Waveshare 1.43C pet firmware, local behavior, classic pink UI, audio and power policies.
- Existing-Bot native macOS voice bridge and task-history recovery.
- Wi-Fi discovery, private-LAN pairing, authenticated BLE and USB provisioning.
- Bundled USB helpers, guarded per-device backup/install/restore, portable optional login-service setup.
- Chinese newcomer guide, English overview, architecture, protocol, tests and explicit acceptance gaps.
- Clean public history; no account/device data, recordings, private backup manifests or precompiled firmware.
