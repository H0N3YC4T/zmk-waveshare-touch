/* The registry of all touch UI views and the build_view() dispatcher.
 * Per-view renderers (build_*) and tap handlers (tap_*) live in views/ --
 * one file per view -- and are declared in touch_ui.h. Navigation
 * (touch_nav.c) only ever consults the registry below. */

#include <zephyr/device.h>
#include "touch_ui.h"

/* ----------------------------- the registry -------------------------------- */

void build_view(enum ui_view v)
{
  const struct view_def *d = &view_defs[v];
  lv_obj_clean(touch_overlay);
  grid_rows = d->rows;
  grid_cols = d->cols;
  if ((ui_rot & 1) && d->rearrange_2x3)
  {
    grid_rows = 3;
    grid_cols = 2;
  }
  if (d->build != NULL)
  {
    d->build();
  }

  /* Armed one-shot modifier -> blue frame visible from any key page. Drawn as its
   * own rounded-rect child (radius = glass) so it follows the screen's physical
   * corners. Cleaned up with the rest of the view. */
  if (pending_mods)
  {
    lv_obj_t *frame = lv_obj_create(touch_overlay);
    if (frame != NULL)
    {
      lv_obj_set_size(frame, scr_w(), scr_h());
      lv_obj_set_pos(frame, 0, 0);
      lv_obj_set_style_bg_opa(frame, LV_OPA_TRANSP, LV_PART_MAIN);
      lv_obj_set_style_border_color(frame, lv_color_hex(COLOR_ACCENT), LV_PART_MAIN);
      lv_obj_set_style_border_width(frame, 3, LV_PART_MAIN);
      lv_obj_set_style_radius(frame, GLASS_RADIUS, LV_PART_MAIN);
      lv_obj_set_style_pad_all(frame, 0, LV_PART_MAIN);
    }
  }
}

const struct view_def view_defs[VIEW_COUNT] = {
    /*                 build            on_tap         r   c   portrait  2x3    tmout  mods   on_hold */
    [VIEW_NORMAL] =    {NULL,           tap_normal,    2,  3,  NULL,     false, false, false, NULL},
    [VIEW_HOME] =      {build_home,     tap_home,      3,  3,  NULL,     false, true,  true,  hold_home},
    [VIEW_SETTINGS] =  {build_settings, tap_settings,  3,  3,  NULL,     false, true,  false, NULL},
    [VIEW_MEDIA] =     {build_media,    tap_media,     2,  3,  p23_pos,  true,  false, true,  NULL},
    [VIEW_FKEYS] =     {build_fkeys,    tap_fkeys,     3,  3,  NULL,     false, false, true,  NULL},
    [VIEW_NUMPAD] =    {build_numpad,   tap_numpad,    4,  4,  NULL,     false, false, true,  NULL},
    [VIEW_SYMBOLS] =   {build_symbols,  tap_symbols,   3,  3,  NULL,     false, false, true,  NULL},
    [VIEW_MODIFIERS] = {build_modifiers,tap_modifiers, 2,  3,  p23_pos,  true,  false, true,  NULL},
    [VIEW_TRACKPAD] =  {build_trackpad, tap_trackpad,  2,  3,  NULL,     false, false, true,  NULL},
    [VIEW_PAD] =       {build_pad,      tap_pad,       2,  3,  p23_pos,  true,  false, true,  NULL},
    [VIEW_CALC] =      {build_calc,     tap_calc,      5,  4,  NULL,     false, false, false, hold_calc},
};
