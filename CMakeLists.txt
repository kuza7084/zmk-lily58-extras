# Sourced into the `app` target itself (not a separate zephyr_library()),
# because app/include is added to `app` with target_include_directories(... PRIVATE),
# so zmk/display/widgets/*.h etc. are only visible to code compiled as part
# of `app` (see caksoylar/zmk-rgbled-widget for the same pattern).
target_sources_ifdef(CONFIG_ZMK_LILY58_EXTRAS_ROTATED_DISPLAY app PRIVATE src/status_screen.c)
