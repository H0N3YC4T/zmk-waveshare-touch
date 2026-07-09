/* -------------------------------- MEDIA ------------------------------------ */
/* Volume / transport, one macro per cell (defined in the consuming keyboard's
 * overlay). Cell 1 = back to HOME. */

#include "../touch_ui.h"

void build_media(void)
{
  draw_cell_l(0, LV_SYMBOL_VOLUME_MID, COLOR_PRIMARY);
  draw_cell_l(1, LV_SYMBOL_UP, COLOR_RED);
  draw_cell_l(2, LV_SYMBOL_VOLUME_MAX, COLOR_PRIMARY);
  draw_cell_l(3, LV_SYMBOL_PREV, COLOR_PRIMARY);
  draw_cell_l(4, LV_SYMBOL_PLAY, COLOR_PRIMARY);
  draw_cell_l(5, LV_SYMBOL_NEXT, COLOR_PRIMARY);
}

void tap_media(int cell)
{
  static const char *const macros[6] = {
      "touch_macro_0",
      NULL /* back */,
      "touch_macro_2",
      "touch_macro_4",
      "touch_macro_3",
      "touch_macro_5",
  };
  if (cell == 1)
  {
    show_view(VIEW_HOME);
  }
  else if (cell >= 0 && cell < 6 && macros[cell] != NULL)
  {
    fire_macro(macros[cell]);
  }
}
