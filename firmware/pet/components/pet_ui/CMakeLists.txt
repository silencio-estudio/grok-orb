# Fuentes según idioma y tema: la CJK solo en chino, y la pixelada solo en el tema de 8 bits.
set(srcs "pet_ui.c" "pet_motion.c")
if(CONFIG_PET_THEME_PIXEL)
    list(APPEND srcs "fonts/pet_font_pixel_16.c" "fonts/pet_font_title_16.c")
else()
    list(APPEND srcs "fonts/pet_font_latin_16.c")
endif()
if(CONFIG_PET_LANG_ZH)
    list(APPEND srcs "fonts/muse_font_cjk_16.c")
endif()
idf_component_register(SRCS ${srcs} INCLUDE_DIRS "."
    REQUIRES pet_core board_port pet_strings PRIV_REQUIRES esp_timer mbedtls esp_driver_usb_serial_jtag)
