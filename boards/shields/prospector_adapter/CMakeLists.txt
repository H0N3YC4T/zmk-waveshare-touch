if(CONFIG_SHIELD_PROSPECTOR_ADAPTER)
  zephyr_library()
  file(GLOB font_sources src/fonts/*.c)
  zephyr_library_sources(${ZEPHYR_BASE}/misc/empty_file.c)
  zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
  zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
  zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_CMAKE_DIR}/include)
  zephyr_library_include_directories(include)
  zephyr_library_sources(src/brightness.c)
  zephyr_library_sources(src/custom_status_screen.c)
  zephyr_library_sources(src/display_rotate_init.c)

  # CST816S gesture driver (taps -> the touch UI, trackpad -> mouse HID). Gated
  # internally on the cst816s DT node, so it compiles to nothing without one.
  zephyr_library_sources(src/touch_input.c)

  if(CONFIG_PROSPECTOR_SHOW_MODIFIERS)
    zephyr_library_sources(src/modifier_order.c)
  endif()

  # Only the OPERATOR layout survives in this module (the touch UI is built on it).
  file(GLOB layout_sources src/layouts/operator/*.c)
  list(FILTER layout_sources EXCLUDE REGEX "status_screen\\.c$")
  zephyr_library_sources(${layout_sources})

  zephyr_library_sources(${font_sources})
endif()