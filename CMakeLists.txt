if(CONFIG_ZMK_LILY58_EXTRAS_ROTATED_DISPLAY)
  zephyr_library()
  zephyr_library_sources(src/status_screen.c)
endif()
