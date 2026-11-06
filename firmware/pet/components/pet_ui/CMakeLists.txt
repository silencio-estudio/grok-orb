# La fuente CJK (Unifont) solo se compila con la interfaz en chino.
set(srcs "pet_ui.c" "fonts/pet_font_latin_16.c")
if(CONFIG_PET_LANG_ZH)
    list(APPEND srcs "fonts/muse_font_cjk_16.c")
endif()
idf_component_register(SRCS ${srcs} INCLUDE_DIRS "."
    REQUIRES pet_core board_port pet_strings PRIV_REQUIRES esp_timer mbedtls esp_driver_usb_serial_jtag)
