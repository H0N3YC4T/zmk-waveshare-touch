/* -------------------------------- HOME ------------------------------------ */

#include <zephyr/device.h>
#include "../touch_ui.h"

static void tap_home_pad(int cell)
{
  if (touch_pad_count() > 0)
  {
    show_view(&view_pad);
  }
}

static void hold_home(int cell)
{
  if (cell == 2)
  {
    show_view(&view_calc);
  }
  else if (cell == 7)
  {
    show_view(&view_clipboard);
  }
  else if (cell == 5)
  {
    show_view(&view_scrollpad);
  }
  else if (cell == 4)
  {
#if DT_NODE_EXISTS(DT_NODELABEL(bootloader))
    fire_macro(DEVICE_DT_NAME(DT_NODELABEL(bootloader)));
#endif
  }
  else
  {
    tap_declarative(cell);
  }
}

static const struct page_cell home_cells[] = {
    {0, 0, 1, 1, NULL, &icon_fkeys, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_fkeys},
    {0, 1, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_normal},
    {0, 2, 1, 1, NULL, &icon_numpad, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_numpad},

    {1, 0, 1, 1, NULL, &icon_symbols, THEME_SECONDARY, ACT_GO_VIEW, .arg.view = &view_symbols},
    {1, 1, 1, 1, NULL, &icon_settings, THEME_FOCUS, ACT_GO_VIEW, .arg.view = &view_settings},
    {1, 2, 1, 1, NULL, &icon_trackpad, THEME_SECONDARY, ACT_GO_VIEW, .arg.view = &view_trackpad},

    {2, 0, 1, 1, "MOD", &icon_modkeys, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_modifiers},
    {2, 1, 1, 1, NULL, &icon_keyboard, THEME_SECONDARY, ACT_CUSTOM, .arg.func = tap_home_pad},
    {2, 2, 1, 1, NULL, &icon_audio, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_media},
    {0}};

const struct view_def view_home = {
    .cells = home_cells,
    .keeps_mods = true,
    .on_hold = hold_home,
    .idle_timeout = true,
};
