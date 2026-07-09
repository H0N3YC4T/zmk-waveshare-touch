/* -------------------------------- HOME ------------------------------------ */
/* 3x3, every screen one tap away (the HUB sub-menu is gone). Cell 7 = the user
 * macro pad (keyboard icon; greyed if the consuming keyboard binds nothing). */

#include <zephyr/device.h>
#include "../touch_ui.h"

void build_home(void)
{
  draw_cell_icon(0, 0, &icon_fkeys, "Fn", COLOR_PRIMARY);
  draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_RED);
  draw_cell_icon(0, 2, &icon_numpad, "123", COLOR_PRIMARY);
  draw_cell_icon(1, 0, &icon_symbols, "#$%", COLOR_PRIMARY);
  draw_cell(1, 1, 1, LV_SYMBOL_SETTINGS, COLOR_PRIMARY);
  draw_cell_icon(1, 2, &icon_trackpad, LV_SYMBOL_GPS, COLOR_PRIMARY);
  draw_cell_icon(2, 0, &icon_modkeys, "MOD", COLOR_PRIMARY);
  draw_cell(2, 1, 1, LV_SYMBOL_KEYBOARD,
            touch_pad_count() > 0 ? COLOR_PRIMARY : COLOR_GREY);
  draw_cell(2, 2, 1, LV_SYMBOL_AUDIO, COLOR_PRIMARY);
}

void tap_home(int cell)
{
  static const enum ui_view targets[9] = {
      VIEW_FKEYS,
      VIEW_NORMAL,
      VIEW_NUMPAD,
      VIEW_SYMBOLS,
      VIEW_SETTINGS,
      VIEW_TRACKPAD,
      VIEW_MODIFIERS,
      VIEW_PAD,
      VIEW_MEDIA,
  };
  if (cell == 7 && touch_pad_count() == 0)
  {
    return; /* nothing bound -- stay put */
  }
  if (cell >= 0 && cell < 9)
  {
    show_view(targets[cell]);
  }
}

/* Long-press shortcuts on HOME: hold 123 -> calculator, hold settings -> the
 * dongle's bootloader (fires ZMK's built-in reset behavior by its DT name, so it
 * does the right thing for whatever bootloader the board is configured with). */
void hold_home(int cell)
{
  if (cell == 2)
  {
    show_view(VIEW_CALC);
  }
  else if (cell == 4)
  {
#if DT_NODE_EXISTS(DT_NODELABEL(bootloader))
    fire_macro(DEVICE_DT_NAME(DT_NODELABEL(bootloader)));
#endif
  }
  else
  {
    tap_home(cell); /* other cells: a hold is just a slow tap */
  }
}
