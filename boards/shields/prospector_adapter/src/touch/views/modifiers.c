/* ------------------------------- MODIFIERS --------------------------------- */
/* One-shot mods; armed = solid blue fill + black text, applied to the next key
 * sent (send_key). Leaving for NORMAL or SETTINGS clears them (keeps_mods). */

#include "../touch_ui.h"

static const uint8_t mod_bits[6] = {MOD_LCTL, 0 /* back */, MOD_LSFT,
                                    MOD_LALT, 0 /* empty */, MOD_LGUI};

static void tap_mod(int cell) {
    if (cell >= 0 && cell < 6 && mod_bits[cell]) {
        pending_mods ^= mod_bits[cell];
        build_view(cur_view);
    }
}

static void tap_mod_0(int cell) { ARG_UNUSED(cell); tap_mod(0); }
static void tap_mod_2(int cell) { ARG_UNUSED(cell); tap_mod(2); }
static void tap_mod_3(int cell) { ARG_UNUSED(cell); tap_mod(3); }
static void tap_mod_5(int cell) { ARG_UNUSED(cell); tap_mod(5); }

static const struct page_cell mod_cells[] = {
    {0, 0, 1, 1, "CTRL", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_0},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_RED, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 2, 1, 1, "SHFT", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_2},
    {1, 0, 1, 1, "ALT", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_3},
    {1, 1, 1, 1, NULL, NULL, COLOR_PRIMARY, ACT_NONE}, // Empty
    {1, 2, 1, 1, "GUI", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_5},
    {0}
};

static const struct page_cell mod_cells_portrait[] = {
    {0, 0, 1, 1, "CTRL", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_0},
    {0, 1, 1, 1, "SHFT", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_2},
    {1, 0, 1, 1, LV_SYMBOL_UP, NULL, COLOR_RED, ACT_GO_VIEW, .arg.view = &view_home},
    {1, 1, 1, 1, NULL, NULL, COLOR_PRIMARY, ACT_NONE}, // Empty
    {2, 0, 1, 1, "ALT", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_3},
    {2, 1, 1, 1, "GUI", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_mod_5},
    {0}
};

static void build_modifiers(void)
{
  int map[] = {0, -1, 2, 3, -1, 5};
  if (ui_rot & 1) {
      map[0] = 0; map[2] = 1; map[3] = 4; map[5] = 5;
  }
  
  for (int c = 0; c < 6; c++) {
      int idx = map[c];
      if (idx >= 0 && cur_view_btns[idx]) {
          bool armed = (pending_mods & mod_bits[c]);
          uint32_t color = armed ? COLOR_ACCENT : COLOR_PRIMARY;
          lv_obj_set_style_bg_color(cur_view_btns[idx], lv_color_hex(armed ? color : COLOR_CHARCOAL), LV_PART_MAIN);
          lv_obj_set_style_border_color(cur_view_btns[idx], lv_color_hex(color), LV_PART_MAIN);
          lv_obj_t *l = lv_obj_get_child(cur_view_btns[idx], 0);
          if (l) lv_obj_set_style_text_color(l, lv_color_hex(armed ? COLOR_BACKGROUND : color), LV_PART_MAIN);
      }
  }
}

const struct view_def view_modifiers = {
    .cells = mod_cells,
    .cells_portrait = mod_cells_portrait,
    .build = build_modifiers,
    .idle_timeout = true,
};
