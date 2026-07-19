/* ------------------------------- MODIFIERS --------------------------------- */

#include "../touch_ui.h"

static const struct page_cell mod_cells[];
static const struct page_cell mod_cells_portrait[];

/* ring colour = the (last) armed mod's own button colour */
static enum theme_role mod_role(int mod)
{
  const struct page_cell *active = (ui_rot & 1) ? mod_cells_portrait : mod_cells;
  for (int i = 0; active[i].row_span != 0 || active[i].col_span != 0; i++)
  {
    if (active[i].action == ACT_CUSTOM_VAL && active[i].arg.custom.val == mod)
    {
      return active[i].color;
    }
  }
  return THEME_SECONDARY;
}

static void tap_mod(int mod)
{
  pending_mods ^= mod;
  if (pending_mods & mod)
  {
    pending_mod_role = mod_role(mod);
  }
  else
  {
    for (int m = 0; m < 8; m++)
    {
      if (pending_mods & BIT(m))
      {
        pending_mod_role = mod_role(BIT(m));
        break;
      }
    }
  }
  build_view(cur_view);
}

static void build_modifiers(void)
{
  const struct page_cell *active = (ui_rot & 1) ? mod_cells_portrait : mod_cells;
  for (int i = 0; active[i].row_span != 0 || active[i].col_span != 0; i++)
  {
    if (active[i].action == ACT_CUSTOM_VAL && cur_view_btns[i])
    {
      uint8_t mod = active[i].arg.custom.val;
      bool armed = (pending_mods & mod);
      enum theme_role color = armed ? active[i].color : THEME_PRIMARY;
      lv_obj_set_style_bg_color(cur_view_btns[i], lv_color_hex(theme_color(armed ? color : THEME_SURFACE)), LV_PART_MAIN);
      lv_obj_set_style_border_color(cur_view_btns[i], lv_color_hex(theme_color(color)), LV_PART_MAIN);
      cell_child_set_color(cur_view_btns[i], armed ? THEME_BACKGROUND : color);
    }
  }
}

static const struct page_cell mod_cells[] = {
    {0, 0, 1, 2, NULL, &icon_alt, THEME_FOCUS, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LALT}},
    {0, 2, 1, 2, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 4, 1, 2, "CTRL", NULL, THEME_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LCTL}},

    {1, 0, 1, 3, NULL, &icon_gui, THEME_DECREMENT, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LGUI}},
    {1, 3, 1, 3, NULL, &icon_shift, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LSFT}},
    {0}};

static const struct page_cell mod_cells_portrait[] = {
    {0, 0, 1, 2, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},

    {1, 0, 1, 1, "CTRL", NULL, THEME_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LCTL}},
    {1, 1, 1, 1, NULL, &icon_alt, THEME_FOCUS, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LALT}},

    {2, 0, 1, 1, NULL, &icon_gui, THEME_DECREMENT, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LGUI}},
    {2, 1, 1, 1, NULL, &icon_shift, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {tap_mod, MOD_LSFT}},
    {0}};

const struct view_def view_modifiers = {
    .cells = mod_cells,
    .cells_portrait = mod_cells_portrait,
    .build = build_modifiers,
    .keeps_mods = true,
};
