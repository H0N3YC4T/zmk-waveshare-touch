/* --------------------------------- MACROS ---------------------------------- */
/* User macro pad: up to 5 buttons bound in the consuming keyboard's keymap via a
 * zmk,prospector-touch-pad node (standard binding syntax -- &kp, &bt, macros).
 * Binding order = cells 0,2,3,4,5. Unbound cells draw greyed, tap = no-op.
 * Faces match the keyboard repo's bindings: $_ terminal / LIST task manager /
 * WIFI browser / EYE_CLOSE show desktop / EDIT notes. */

#include "../touch_ui.h"

void build_pad(void)
{
  static const char *const lbls[6] = {"$_", NULL,
                                      LV_SYMBOL_LIST, LV_SYMBOL_WIFI,
                                      LV_SYMBOL_EYE_CLOSE, LV_SYMBOL_EDIT};
  int n = touch_pad_count();
  draw_cell_l(1, LV_SYMBOL_UP, COLOR_RED);
  for (int c = 0, i = 0; c < 6; c++)
  {
    if (lbls[c] != NULL)
    {
      draw_cell_l(c, lbls[c], i < n ? COLOR_PRIMARY : COLOR_GREY);
      i++;
    }
  }
}

void tap_pad(int cell)
{
  if (cell == 1)
  {
    show_view(VIEW_HOME);
  }
  else if (cell >= 0 && cell < 6)
  {
    fire_pad(cell == 0 ? 0 : cell - 1); /* cells 0,2,3,4,5 -> M1..M5 */
  }
}
