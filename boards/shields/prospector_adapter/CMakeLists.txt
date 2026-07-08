if(CONFIG_SHIELD_PROSPECTOR_ADAPTER)
  zephyr_library()
  zephyr_library_sources(${ZEPHYR_BASE}/misc/empty_file.c)
  zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
  zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
  zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_CMAKE_DIR}/include)
  zephyr_library_include_directories(include src/touch src/widgets)

  # Status screen entry point + supporting features
  zephyr_library_sources(src/status_screen.c)
  zephyr_library_sources(src/brightness.c)
  zephyr_library_sources(src/display_rotate_init.c)

  if(CONFIG_PROSPECTOR_SHOW_MODIFIERS)
    zephyr_library_sources(src/modifier_order.c)
  endif()

  # Touch UI, one feature per file (see src/touch/touch_ui.h). touch_input.c is
  # the CST816S gesture driver, internally gated on the cst816s DT node.
  file(GLOB touch_sources src/touch/*.c)
  zephyr_library_sources(${touch_sources})

  # Status-screen widgets (OPERATOR layout)
  file(GLOB widget_sources src/widgets/*.c)
  zephyr_library_sources(${widget_sources})

  file(GLOB font_sources src/fonts/*.c)
  zephyr_library_sources(${font_sources})

  # Optional custom icons (weak symbols -- absent files just mean text fallbacks).
  # Guarded: an empty glob would call zephyr_library_sources() with no args = error.
  file(GLOB icon_sources src/icons/*.c)
  if(icon_sources)
    zephyr_library_sources(${icon_sources})
  endif()
endif()
