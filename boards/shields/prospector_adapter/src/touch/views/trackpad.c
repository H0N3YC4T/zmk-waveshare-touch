/* ------------------------------- TRACKPAD ---------------------------------- */
/* Whole screen = trackpad (gestures in touch_input.c): drag = move pointer,
 * 1 tap = L-click, 2 taps = R-click, tap-then-hold-and-drag = drag-lock,
 * top-left corner = exit. Scroll lane = logical coord >= TP_SCROLL_ZONE
 * (touch_ui.h, shared with the gesture boundary in touch_input.c) along the
 * long axis, drawn flush to the edge: right-side vertical strip in landscape,
 * bottom horizontal strip in portrait (swipe right = scroll down). */

#include "../touch_ui.h"

void build_trackpad(void)
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
    lv_label_set_text(ex, LV_SYMBOL_CLOSE); /* X: exit the pad, not a nav level */
    lv_obj_set_style_text_font(ex, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(ex, lv_color_hex(COLOR_RED), LV_PART_MAIN);
    lv_obj_set_pos(ex, 16, 12); /* corner-exit affordance, clear of the glass arc */
  }
  lv_obj_t *hint = lv_label_create(touch_overlay);
  if (hint != NULL)
  {
    lv_label_set_text(hint, "TRACKPAD");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(hint, lv_color_hex(COLOR_DARK_GREY), LV_PART_MAIN);
    /* centred on the pad area (excluding the lane) */
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

void tap_trackpad(int cell)
{
  /* touch_input.c only forwards the corner-exit tap here. */
  ARG_UNUSED(cell);
  show_view(VIEW_HOME);
}
