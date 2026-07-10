/* ------------------------------- MODIFIERS --------------------------------- */


#include "../touch_ui.h"

static void tap_mod(int mod) {
    pending_mods ^= mod;
    build_view(cur_view);
}

static const struct page_cell mod_cells[];
static const struct page_cell mod_cells_portrait[];

static void build_modifiers(void)
{
  const struct page_cell *active = (ui_rot & 1) ? mod_cells_portrait : mod_cells;
  for (int i = 0; active[i].row_span != 0 || active[i].col_span != 0; i++) {
      if (active[i].action == ACT_CUSTOM_VAL && cur_view_btns[i]) {
          uint8_t mod = active[i].arg.custom.val;
          bool armed = (pending_mods & mod);
          uint32_t color = armed ? COLOR_ACCENT : COLOR_PRIMARY;
          lv_obj_set_style_bg_color(cur_view_btns[i], lv_color_hex(armed ? color : COLOR_CHARCOAL), LV_PART_MAIN);
          lv_obj_set_style_border_color(cur_view_btns[i], lv_color_hex(color), LV_PART_MAIN);
          lv_obj_t *l = lv_obj_get_child(cur_view_btns[i], 0);
          if (l) lv_obj_set_style_text_color(l, lv_color_hex(armed ? COLOR_BACKGROUND : color), LV_PART_MAIN);
      }
  }
}

static const struct page_cell mod_cells[] = {
    {0, 0, 1, 1, "CTRL", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LCTL}},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_RED, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 2, 1, 1, "SHFT", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LSFT}},
    {1, 0, 1, 1, "ALT", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LALT}},
    {1, 1, 1, 1, NULL, NULL, COLOR_PRIMARY, ACT_NONE}, // Empty
    {1, 2, 1, 1, "GUI", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LGUI}},
    {0}
};

static const struct page_cell mod_cells_portrait[] = {
    {0, 0, 1, 1, "CTRL", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LCTL}},
    {0, 1, 1, 1, "SHFT", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LSFT}},
    {1, 0, 1, 1, LV_SYMBOL_UP, NULL, COLOR_RED, ACT_GO_VIEW, .arg.view = &view_home},
    {1, 1, 1, 1, NULL, NULL, COLOR_PRIMARY, ACT_NONE}, // Empty
    {2, 0, 1, 1, "ALT", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LALT}},
    {2, 1, 1, 1, "GUI", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LGUI}},
    {0}
};



const struct view_def view_modifiers = {
    .cells = mod_cells,
    .cells_portrait = mod_cells_portrait,
    .build = build_modifiers,
    .keeps_mods = true,
    .idle_timeout = true,
};
