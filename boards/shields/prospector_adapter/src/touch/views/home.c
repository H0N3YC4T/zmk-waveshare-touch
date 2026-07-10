/* -------------------------------- HOME ------------------------------------ */
/* 3x3, every screen one tap away (the HUB sub-menu is gone). Cell 7 = the user
 * macro pad (keyboard icon; greyed if the consuming keyboard binds nothing). */

#include <zephyr/device.h>
#include "../touch_ui.h"

static void tap_home_pad(int cell)
{
  if (touch_pad_count() > 0)
  {
    show_view(&view_pad);
  }
}



/* Long-press shortcuts on HOME: hold 123 -> calculator, hold settings -> the
 * dongle's bootloader (fires ZMK's built-in reset behavior by its DT name, so it
 * does the right thing for whatever bootloader the board is configured with). */
static void hold_home(int cell)
{
  if (cell == 2)
  {
    show_view(&view_calc);
  }
  else if (cell == 4)
  {
#if DT_NODE_EXISTS(DT_NODELABEL(bootloader))
    fire_macro(DEVICE_DT_NAME(DT_NODELABEL(bootloader)));
#endif
  }
  else
  {
    tap_declarative(cell); /* other cells: a hold is just a slow tap */
  }
}

static const struct page_cell home_cells[] = {
    {0, 0, 1, 1, "Fn",               &icon_fkeys,    COLOR_PRIMARY, ACT_GO_VIEW, .arg.view = &view_fkeys},
    {0, 1, 1, 1, LV_SYMBOL_UP,       NULL,           COLOR_RED,     ACT_GO_VIEW, .arg.view = &view_normal},
    {0, 2, 1, 1, "123",              &icon_numpad,   COLOR_PRIMARY, ACT_GO_VIEW, .arg.view = &view_numpad},
    {1, 0, 1, 1, "#$%",              &icon_symbols,  COLOR_PRIMARY, ACT_GO_VIEW, .arg.view = &view_symbols},
    {1, 1, 1, 1, LV_SYMBOL_SETTINGS, NULL,           COLOR_PRIMARY, ACT_GO_VIEW, .arg.view = &view_settings},
    {1, 2, 1, 1, LV_SYMBOL_GPS,      &icon_trackpad, COLOR_PRIMARY, ACT_GO_VIEW, .arg.view = &view_trackpad},
    {2, 0, 1, 1, "MOD",              &icon_modkeys,  COLOR_PRIMARY, ACT_GO_VIEW, .arg.view = &view_modifiers},
    {2, 1, 1, 1, LV_SYMBOL_KEYBOARD, NULL,           COLOR_PRIMARY, ACT_CUSTOM,  .arg.func = tap_home_pad},
    {2, 2, 1, 1, LV_SYMBOL_AUDIO,    NULL,           COLOR_PRIMARY, ACT_GO_VIEW, .arg.view = &view_media},
    {0}
};

const struct view_def view_home = {
    .cells = home_cells,
    .keeps_mods = true,
    .on_hold = hold_home,
};
