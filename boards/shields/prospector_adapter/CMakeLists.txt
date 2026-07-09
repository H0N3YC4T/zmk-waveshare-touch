if(CONFIG_SHIELD_PROSPECTOR_ADAPTER)
  zephyr_library()
  zephyr_library_sources(${ZEPHYR_BASE}/misc/empty_file.c)
  zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
  zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
  zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_CMAKE_DIR}/include)
  zephyr_library_include_directories(include src src/touch src/widgets)

  # Status screen entry point + supporting features
  zephyr_library_sources(src/status_screen.c)
  zephyr_library_sources(src/brightness.c)
  zephyr_library_sources(src/display_rotate_init.c)

  if(CONFIG_PROSPECTOR_SHOW_MODIFIERS)
    zephyr_library_sources(src/modifier_order.c)
  endif()

  # Touch UI (see src/touch/touch_ui.h):
  #   touch_main.c        -- view registry + build_view() dispatcher
  #   tools/touch_draw.c  -- grid geometry + button drawing
  #   tools/touch_input.c -- CST816S gesture driver (gated on cst816s DT node)
  #   tools/touch_keys.c  -- workqueue key/macro sending (thread-safety boundary)
  #   tools/touch_nav.c   -- tap routing, view state, idle timeout
  #   tools/touch_rotation.c -- 4-step display rotation
  #   views/              -- per-view renderers (one file per view)
  file(GLOB touch_sources src/touch/*.c)
  file(GLOB tool_sources src/touch/tools/*.c)
  file(GLOB view_sources src/touch/views/*.c)
  zephyr_library_sources(${touch_sources})
  zephyr_library_sources(${tool_sources})
  zephyr_library_sources(${view_sources})

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
