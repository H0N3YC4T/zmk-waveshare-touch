/* ------------------------------- SETTINGS --------------------------------- */
/* 3x3. One setting per outer column: left = trackpad sensitivity, right = display
 * brightness. Rows 0/1 = + (green) and - (yellow), greyed at the end stop; row 2 =
 * icon + value readouts (purple, not keys) splitting the row 50/50. Middle: back /
 * rotate -- the rotate button is the only blue on the screen, so it stands out.
 * (No sun glyph exists in LVGL's built-in set; eye-open marks brightness.) */

#include "../touch_ui.h"

void build_settings(void)
{
  char lbl[20];
  int br = prospector_brightness_get();
  int sn = prospector_touchpad_sens_get();
  if (sn >= 0)
  {
    draw_cell(0, 0, 1, LV_SYMBOL_PLUS,
              sn >= SETTINGS_SENS_MAX ? COLOR_GREY : COLOR_GREEN);
    draw_cell(1, 0, 1, LV_SYMBOL_MINUS, sn <= 0 ? COLOR_GREY : COLOR_YELLOW);
  }
  draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_RED);
  draw_cell(1, 1, 1, LV_SYMBOL_REFRESH, COLOR_ACCENT); /* rotate 90deg cw per tap */
  draw_cell(0, 2, 1, LV_SYMBOL_PLUS,
            br >= SETTINGS_BRIGHT_MAX ? COLOR_GREY : COLOR_GREEN);
  draw_cell(1, 2, 1, LV_SYMBOL_MINUS,
            br <= SETTINGS_BRIGHT_MIN ? COLOR_GREY : COLOR_YELLOW);
  /* Readout row on a temporary 2-column grid (1.5 units each); taps still
   * resolve on the 3x3 grid -- row 2 is no-op cells either way. */
  grid_cols = 2;
  if (sn >= 0)
  {
    lv_snprintf(lbl, sizeof(lbl), LV_SYMBOL_GPS " %d", sn);
    draw_cell(2, 0, 1, lbl, COLOR_PRIMARY);
  }
  lv_snprintf(lbl, sizeof(lbl), LV_SYMBOL_EYE_OPEN " %d%%", br);
  draw_cell(2, 1, 1, lbl, COLOR_PRIMARY);
  grid_cols = 3;
}

void tap_settings(int cell)
{
  switch (cell)
  {
  case 0:
    prospector_touchpad_sens_step(+1);
    build_view(VIEW_SETTINGS);
    break;
  case 1:
    show_view(VIEW_HOME);
    break;
  case 2:
    prospector_brightness_step(+BRIGHTNESS_STEP);
    build_view(VIEW_SETTINGS);
    break;
  case 3:
    prospector_touchpad_sens_step(-1);
    build_view(VIEW_SETTINGS);
    break;
  case 4:
    ui_rot = (ui_rot + 1) & 3;
    settings_apply_rotation();
    break; /* +90deg CW */
  case 5:
    prospector_brightness_step(-BRIGHTNESS_STEP);
    build_view(VIEW_SETTINGS);
    break;
  default:
    break; /* cells 6-8: the readout row */
  }
}
