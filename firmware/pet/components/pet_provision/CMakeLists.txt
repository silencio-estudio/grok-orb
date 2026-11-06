idf_component_register(SRCS "pet_provision.c" "pet_setup_guard.c" INCLUDE_DIRS "." REQUIRES pet_network PRIV_REQUIRES pet_strings bt esp_timer cjson)
