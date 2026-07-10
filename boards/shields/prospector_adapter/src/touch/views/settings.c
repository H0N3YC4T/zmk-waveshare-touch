/* ------------------------------- SETTINGS --------------------------------- */
/* 3x3. One setting per outer column: left = trackpad sensitivity, right = display
 * brightness. Rows 0/1 = + (green) and - (yellow), greyed at the end stop; row 2 =
 * icon + value readouts (purple, not keys) splitting the row 50/50. Middle: back /
 * rotate -- the rotate button is the only blue on the screen, so it stands out.
 * (No sun glyph exists in LVGL's built-in set; eye-open marks brightness.) */

#include "../touch_ui.h"

static void build_settings(void);

static void tap_sens(int delta) { prospector_touchpad_sens_step(delta); build_settings(); }
static void tap_bright(int delta) { prospector_brightness_step(delta); build_settings(); }
static void tap_rotate(int cell) { ARG_UNUSED(cell); ui_rot = (ui_rot + 1) & 3; settings_apply_rotation(); }

static void build_settings(void)
{
  int br = prospector_brightness_get();
  int sn = prospector_touchpad_sens_get();

  if (sn >= 0) {
      if (cur_view_btns[0]) lv_obj_set_style_border_color(cur_view_btns[0], lv_color_hex(sn >= SETTINGS_SENS_MAX ? COLOR_GREY : COLOR_GREEN), LV_PART_MAIN);
      if (cur_view_btns[3]) lv_obj_set_style_border_color(cur_view_btns[3], lv_color_hex(sn <= 0 ? COLOR_GREY : COLOR_ALERT), LV_PART_MAIN);
      if (cur_view_btns[6]) {
          lv_obj_t *l = lv_obj_get_child(cur_view_btns[6], 0);
          if (l) lv_label_set_text_fmt(l, LV_SYMBOL_GPS " %d", sn);
      }
  } else {
      if (cur_view_btns[0]) lv_obj_add_flag(cur_view_btns[0], LV_OBJ_FLAG_HIDDEN);
      if (cur_view_btns[3]) lv_obj_add_flag(cur_view_btns[3], LV_OBJ_FLAG_HIDDEN);
      if (cur_view_btns[6]) lv_obj_add_flag(cur_view_btns[6], LV_OBJ_FLAG_HIDDEN);
  }

  if (cur_view_btns[2]) lv_obj_set_style_border_color(cur_view_btns[2], lv_color_hex(br >= SETTINGS_BRIGHT_MAX ? COLOR_GREY : COLOR_GREEN), LV_PART_MAIN);
  if (cur_view_btns[5]) lv_obj_set_style_border_color(cur_view_btns[5], lv_color_hex(br <= SETTINGS_BRIGHT_MIN ? COLOR_GREY : COLOR_ALERT), LV_PART_MAIN);
  if (cur_view_btns[7]) {
      lv_obj_t *l = lv_obj_get_child(cur_view_btns[7], 0);
      if (l) lv_label_set_text_fmt(l, LV_SYMBOL_EYE_OPEN " %d%%", br);
  }
}

static const struct page_cell settings_cells[] = {
    {0, 0, 1, 2, LV_SYMBOL_PLUS, NULL, COLOR_GREEN, ACT_CUSTOM_VAL, .arg.custom = {tap_sens, +1}},
    {0, 2, 1, 2, LV_SYMBOL_UP, NULL, COLOR_RED, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 4, 1, 2, LV_SYMBOL_PLUS, NULL, COLOR_GREEN, ACT_CUSTOM_VAL, .arg.custom = {tap_bright, +BRIGHTNESS_STEP}},
    {1, 0, 1, 2, LV_SYMBOL_MINUS, NULL, COLOR_ALERT, ACT_CUSTOM_VAL, .arg.custom = {tap_sens, -1}},
    {1, 2, 1, 2, LV_SYMBOL_REFRESH, NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_rotate},
    {1, 4, 1, 2, LV_SYMBOL_MINUS, NULL, COLOR_ALERT, ACT_CUSTOM_VAL, .arg.custom = {tap_bright, -BRIGHTNESS_STEP}},
    {2, 0, 1, 3, " ", NULL, COLOR_PRIMARY, ACT_NONE},
    {2, 3, 1, 3, " ", NULL, COLOR_PRIMARY, ACT_NONE},
    {0}
};

const struct view_def view_settings = {
    .cells = settings_cells,
    .build = build_settings,
    .keeps_mods = true,
};
