/* ------------------------------- STATUS ----------------------------------- */
/* The stock status screen; the overlay is hidden. Any tap opens the menu. */

#include "../touch_ui.h"

void tap_normal(int cell)
{
  ARG_UNUSED(cell);
  show_view(VIEW_HOME);
}
