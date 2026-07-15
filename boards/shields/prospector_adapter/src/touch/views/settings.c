/* ------------------------------- SETTINGS --------------------------------- */


#include <stdio.h>
#include "../touch_ui.h"

static void build_settings(void);

static void tap_sens(int delta) { prospector_touchpad_sens_step(delta); build_settings(); }
static void tap_bright(int delta) { prospector_brightness_step(delta); build_settings(); }
static void tap_rotate(int cell) { ARG_UNUSED(cell); ui_rot = (ui_rot + 1) & 3; settings_apply_rotation(); }

#define SET_BTN_SENS_READOUT 0
#define SET_BTN_BRIGHT_READOUT 2
#define SET_BTN_SENS_PLUS 3
#define SET_BTN_BRIGHT_PLUS 5
#define SET_BTN_SENS_MINUS 6
#define SET_BTN_BRIGHT_MINUS 8

/* icon + number readout; buttons are recreated per build_view, but this also
   runs standalone after taps -> find existing children instead of re-adding */
static void readout_set(int idx, const lv_image_dsc_t *icon, const char *text)
{
  lv_obj_t *btn = cur_view_btns[idx];
  if (btn == NULL)
  {
    return;
  }
  lv_obj_t *img = NULL, *lbl = NULL;
  for (uint32_t i = 0; i < lv_obj_get_child_count(btn); i++)
  {
    lv_obj_t *ch = lv_obj_get_child(btn, i);
    if (lv_obj_check_type(ch, &lv_image_class)) img = ch;
    else if (lv_obj_check_type(ch, &lv_label_class)) lbl = ch;
  }
  if (img == NULL)
  {
    img = lv_image_create(btn);
    if (img != NULL)
    {
      lv_image_set_src(img, icon);
      lv_obj_set_style_image_recolor(img, lv_color_hex(theme_color(THEME_PRIMARY)), LV_PART_MAIN);
      lv_obj_set_style_image_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_move_to_index(img, 0);
    }
    if (lbl != NULL)
    {
      /* 1x1 readout cell: smaller face so icon + "100%" fit */
      lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, LV_PART_MAIN);
    }
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn, 6, LV_PART_MAIN);
  }
  if (lbl != NULL)
  {
    lv_label_set_text(lbl, text);
  }
}

static void build_settings(void)
{
  int br = prospector_brightness_get();
  int sn = prospector_touchpad_sens_get();

  if (sn >= 0) {
      if (cur_view_btns[SET_BTN_SENS_PLUS]) lv_obj_set_style_border_color(cur_view_btns[SET_BTN_SENS_PLUS], lv_color_hex(theme_color(sn >= SETTINGS_SENS_MAX ? THEME_MUTED : THEME_ACCEPT)), LV_PART_MAIN);
      if (cur_view_btns[SET_BTN_SENS_MINUS]) lv_obj_set_style_border_color(cur_view_btns[SET_BTN_SENS_MINUS], lv_color_hex(theme_color(sn <= 0 ? THEME_MUTED : THEME_FOCUS)), LV_PART_MAIN);
      if (cur_view_btns[SET_BTN_SENS_READOUT]) {
          static char sens_text[8];
          snprintf(sens_text, sizeof(sens_text), "%d", sn);
          readout_set(SET_BTN_SENS_READOUT, &icon_sens, sens_text);
      }
  } else {
      if (cur_view_btns[SET_BTN_SENS_PLUS]) lv_obj_add_flag(cur_view_btns[SET_BTN_SENS_PLUS], LV_OBJ_FLAG_HIDDEN);
      if (cur_view_btns[SET_BTN_SENS_MINUS]) lv_obj_add_flag(cur_view_btns[SET_BTN_SENS_MINUS], LV_OBJ_FLAG_HIDDEN);
      if (cur_view_btns[SET_BTN_SENS_READOUT]) lv_obj_add_flag(cur_view_btns[SET_BTN_SENS_READOUT], LV_OBJ_FLAG_HIDDEN);
  }

  if (cur_view_btns[SET_BTN_BRIGHT_PLUS]) lv_obj_set_style_border_color(cur_view_btns[SET_BTN_BRIGHT_PLUS], lv_color_hex(theme_color(br >= SETTINGS_BRIGHT_MAX ? THEME_MUTED : THEME_ACCEPT)), LV_PART_MAIN);
  if (cur_view_btns[SET_BTN_BRIGHT_MINUS]) lv_obj_set_style_border_color(cur_view_btns[SET_BTN_BRIGHT_MINUS], lv_color_hex(theme_color(br <= SETTINGS_BRIGHT_MIN ? THEME_MUTED : THEME_FOCUS)), LV_PART_MAIN);
  if (cur_view_btns[SET_BTN_BRIGHT_READOUT]) {
      static char bright_text[8];
      snprintf(bright_text, sizeof(bright_text), "%d%%", br);
      readout_set(SET_BTN_BRIGHT_READOUT, &icon_bright, bright_text);
  }
}

static const struct page_cell settings_cells[] = {
    {0, 0, 1, 1, " ", NULL, THEME_PRIMARY, ACT_NONE},
    {0, 1, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 2, 1, 1, " ", NULL, THEME_PRIMARY, ACT_NONE},
    {1, 0, 1, 1, NULL, &icon_plus, THEME_ACCEPT, ACT_CUSTOM_VAL, .arg.custom = {tap_sens, +1}},
    {1, 1, 1, 1, NULL, &icon_rotate, THEME_SECONDARY, ACT_CUSTOM, .arg.func = tap_rotate},
    {1, 2, 1, 1, NULL, &icon_plus, THEME_ACCEPT, ACT_CUSTOM_VAL, .arg.custom = {tap_bright, +BRIGHTNESS_STEP}},
    {2, 0, 1, 1, NULL, &icon_minus, THEME_FOCUS, ACT_CUSTOM_VAL, .arg.custom = {tap_sens, -1}},
    {2, 1, 1, 1, NULL, &icon_theme, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_theme},
    {2, 2, 1, 1, NULL, &icon_minus, THEME_FOCUS, ACT_CUSTOM_VAL, .arg.custom = {tap_bright, -BRIGHTNESS_STEP}},
    {0}
};

const struct view_def view_settings = {
    .cells = settings_cells,
    .build = build_settings,
    .keeps_mods = true,
};
