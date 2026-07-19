/* Grid geometry + button drawing for the touch UI, */

#include "../touch_ui.h"

int grid_rows = 2; /* rows in the current screen's grid */
int grid_cols = 3; /* cols in the current screen's grid */

static lv_obj_t *make_btn(int row, int col,
                          int w_cells, int h_cells,
                          enum theme_role accent,
                          int pct,
                          bool filled)
{
  lv_coord_t cwc = (scr_w() - 2 * UI_PAD) / grid_cols;
  lv_coord_t ch = (scr_h() - 2 * UI_PAD) / grid_rows;
  lv_coord_t cw = cwc * w_cells;
  lv_coord_t ch_total = ch * h_cells;
  lv_coord_t pad_w = cwc * (100 - pct) / 200;
  lv_coord_t pad_h = ch * 20 / 200; /* 80% default means 10% padding per side */
  lv_coord_t bw = cw - 2 * pad_w;
  lv_coord_t bh = ch_total - 2 * pad_h;

  lv_obj_t *b = lv_obj_create(touch_overlay);
  if (b == NULL)
  {
    return NULL; /* LVGL pool exhausted -- skip this cell rather than deref NULL */
  }
  lv_obj_set_size(b, bw, bh);
  lv_obj_set_pos(b, UI_PAD + col * cwc + (cw - bw) / 2, UI_PAD + row * ch + (ch_total - bh) / 2);
  lv_obj_set_style_bg_color(b, lv_color_hex(theme_color(filled ? accent : THEME_SURFACE)), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(b, lv_color_hex(theme_color(accent)), LV_PART_MAIN);
  lv_obj_set_style_border_width(b, 2, LV_PART_MAIN);
  lv_obj_set_style_radius(b, BTN_RADIUS, LV_PART_MAIN);
  lv_obj_set_style_pad_all(b, 0, LV_PART_MAIN);
  return b;
}

static lv_obj_t *draw_cell_impl(int row, int col,
                                int w_cells, int h_cells,
                                const char *text,
                                enum theme_role accent,
                                int pct,
                                bool filled)
{
  lv_obj_t *b = make_btn(row, col, w_cells, h_cells, accent, pct, filled);
  if (b == NULL)
  {
    return NULL;
  }
  lv_obj_t *l = lv_label_create(b);
  if (l == NULL)
  {
    return b;
  }
  lv_label_set_text(l, text);
  lv_obj_set_style_text_font(l, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_set_style_text_color(l, lv_color_hex(theme_color(filled ? THEME_BACKGROUND : accent)), LV_PART_MAIN);

  lv_obj_set_style_text_letter_space(l, 1, LV_PART_MAIN);
  lv_obj_center(l);
  return b;
}

lv_obj_t *draw_cell_ext(int row, int col,
                        int w_cells, int h_cells,
                        const char *text,
                        enum theme_role accent,
                        bool filled)
{
  return draw_cell_impl(row, col, w_cells, h_cells, text, accent, 80, filled);
}

lv_obj_t *draw_cell_icon_ext(int row, int col,
                             int w_cells, int h_cells,
                             const lv_image_dsc_t *icon,
                             const char *fallback,
                             enum theme_role accent)
{
  if (icon == NULL)
  {
    return draw_cell_ext(row, col, w_cells, h_cells, fallback, accent, false);
  }
  lv_obj_t *b = make_btn(row, col, w_cells, h_cells, accent, 80, false);
  if (b == NULL)
  {
    return NULL;
  }
  lv_obj_t *img = lv_image_create(b);
  if (img == NULL)
  {
    return NULL;
  }
  lv_image_set_src(img, icon);
  lv_obj_set_style_image_recolor(img, lv_color_hex(theme_color(accent)), LV_PART_MAIN);
  lv_obj_set_style_image_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_center(img);
  return b;
}

/* recolor a cell's first child whether it is a text label or an icon image */
void cell_child_set_color(lv_obj_t *btn, enum theme_role role)
{
  if (btn == NULL)
  {
    return;
  }
  lv_obj_t *ch = lv_obj_get_child(btn, 0);
  if (ch == NULL)
  {
    return;
  }
  if (lv_obj_check_type(ch, &lv_image_class))
  {
    lv_obj_set_style_image_recolor(ch, lv_color_hex(theme_color(role)), LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(ch, LV_OPA_COVER, LV_PART_MAIN);
  }
  else if (lv_obj_check_type(ch, &lv_label_class))
  {
    lv_obj_set_style_text_color(ch, lv_color_hex(theme_color(role)), LV_PART_MAIN);
  }
}
