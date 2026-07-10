/* ------------------------------- TRACKPAD ---------------------------------- */


#include "../touch_ui.h"

static void build_trackpad(void)
{
  lv_obj_t *lane = lv_obj_create(touch_overlay);
  if (lane != NULL)
  {
    if (ui_rot & 1)
    {
      lv_obj_set_size(lane, scr_w(), scr_h() - TP_SCROLL_ZONE);
      lv_obj_set_pos(lane, 0, TP_SCROLL_ZONE);
    }
    else
    {
      lv_obj_set_size(lane, scr_w() - TP_SCROLL_ZONE, scr_h());
      lv_obj_set_pos(lane, TP_SCROLL_ZONE, 0);
    }
    lv_obj_set_style_bg_color(lane, lv_color_hex(COLOR_NEAR_BLACK), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lane, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(lane, lv_color_hex(COLOR_SLATE), LV_PART_MAIN);
    lv_obj_set_style_border_width(lane, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(lane, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(lane, 0, LV_PART_MAIN);
  }
  lv_obj_t *ex = lv_label_create(touch_overlay);
  if (ex != NULL)
  {
    lv_label_set_text(ex, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_font(ex, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(ex, lv_color_hex(COLOR_RED), LV_PART_MAIN);
    lv_obj_set_pos(ex, 16, 12);
  }
  lv_obj_t *hint = lv_label_create(touch_overlay);
  if (hint != NULL)
  {
    lv_label_set_text(hint, "TRACKPAD");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(hint, lv_color_hex(COLOR_DARK_GREY), LV_PART_MAIN);
    if (ui_rot & 1)
    {
      lv_obj_align(hint, LV_ALIGN_CENTER, 0, -20);
    }
    else
    {
      lv_obj_align(hint, LV_ALIGN_CENTER, -20, 0);
    }
  }
}

static const struct page_cell trackpad_cells[] = {
    {0, 0, 1, 1, NULL, NULL, 0, ACT_GO_VIEW, .arg.view = &view_home},
    {5, 6, 1, 1, NULL, NULL, 0, ACT_NONE},
    {0}
};

static const struct page_cell trackpad_cells_portrait[] = {
    {0, 0, 1, 1, NULL, NULL, 0, ACT_GO_VIEW, .arg.view = &view_home},
    {6, 5, 1, 1, NULL, NULL, 0, ACT_NONE},
    {0}
};

const struct view_def view_trackpad = {
    .cells = trackpad_cells,
    .cells_portrait = trackpad_cells_portrait,
    .build = build_trackpad,
    .keeps_mods = true,
};
