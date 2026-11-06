#pragma once
#include <stddef.h>
#include <stdint.h>
/* En el simulador el volcado por USB de pet_ui_request_snapshot() se descarta. */
static inline int usb_serial_jtag_write_bytes(const void *bytes,size_t size,uint32_t ticks){(void)bytes;(void)ticks;return (int)size;}
