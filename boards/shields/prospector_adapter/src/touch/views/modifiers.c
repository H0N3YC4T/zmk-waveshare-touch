/* ------------------------------- MODIFIERS --------------------------------- */
/* One-shot mods; armed = solid blue fill + black text, applied to the next key
 * sent (send_key). Leaving for NORMAL or SETTINGS clears them (keeps_mods). */

#include "../touch_ui.h"

static const uint8_t mod_bits[6] = {MOD_LCTL, 0 /* back */, MOD_LSFT,
                                    MOD_LALT, 0 /* empty */, MOD_LGUI};
static const char *const mod_lbls[6] = {"CTRL", NULL, "SHFT", "ALT", NULL, "GUI"};

void build_modifiers(void)
{
  draw_cell_l(1, LV_SYMBOL_UP, COLOR_RED);
  for (int c = 0; c < 6; c++)
  {
    if (mod_lbls[c] == NULL)
    {
      continue;
    }
    if (pending_mods & mod_bits[c])
    {
      draw_cell_on_l(c, mod_lbls[c], COLOR_ACCENT);
    }
    else
    {
      draw_cell_l(c, mod_lbls[c], COLOR_PRIMARY);
    }
  }
}

void tap_modifiers(int cell)
{
  if (cell == 1)
  {
    show_view(VIEW_HOME);
  }
  else if (cell >= 0 && cell < 6 && mod_bits[cell])
  {
    pending_mods ^= mod_bits[cell];
    build_view(VIEW_MODIFIERS);
  }
}
