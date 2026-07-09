/* Grid geometry + button draawing for the touch UI, */

#include "../touch_ui.h"

int grid_rows = 2; /* rows in the current screen's grid (2 / 3 / 4) */
int grid_cols = 3; /* cols in the current screen's grid (2 / 3 / 4) */

/* 3x3 paginated layout: 7 key cells (all but 1 = nav-top and 7 = nav-bottom). */
const int key_cells[KEYS_PER_PAGE] = {0, 2, 3, 4, 5, 6, 8};

/* One button: `pct`% of its cell width (centred), rounded, charcoal fill, accent
 * border+text; `filled` inverts (solid accent, black text) for armed/on states.
 * The grid lives inside a UI_PAD safe inset so corner buttons clear the glass arcs.
 * draw_cell() is the outline 80% default; wider variants give long labels room. */
static lv_obj_t *make_btn(int row, int col, int w_cells, int h_cells, uint32_t accent, int pct, bool filled)
{
  lv_coord_t cwc = (scr_w() - 2 * UI_PAD) / grid_cols;
  lv_coord_t ch = (scr_h() - 2 * UI_PAD) / grid_rows;
  lv_coord_t cw = cwc * w_cells;
  lv_coord_t ch_total = ch * h_cells;
  lv_coord_t bw = cw * pct / 100, bh = ch_total * 4 / 5;

  lv_obj_t *b = lv_obj_create(touch_overlay);
  if (b == NULL)
  {
    return NULL; /* LVGL pool exhausted -- skip this cell rather than deref NULL */
  }
  lv_obj_set_size(b, bw, bh);
  lv_obj_set_pos(b, UI_PAD + col * cwc + (cw - bw) / 2, UI_PAD + row * ch + (ch_total - bh) / 2);
  lv_obj_set_style_bg_color(b, lv_color_hex(filled ? accent : COLOR_CHARCOAL), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(b, lv_color_hex(accent), LV_PART_MAIN);
  lv_obj_set_style_border_width(b, 2, LV_PART_MAIN);
  lv_obj_set_style_radius(b, BTN_RADIUS, LV_PART_MAIN);
  lv_obj_set_style_pad_all(b, 0, LV_PART_MAIN);
  return b;
}

static lv_obj_t *draw_cell_impl(int row, int col, int w_cells, int h_cells, const char *text, uint32_t accent,
                           int pct, bool filled)
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
  lv_obj_set_style_text_color(l, lv_color_hex(filled ? COLOR_BACKGROUND : accent), LV_PART_MAIN);

  lv_obj_set_style_text_letter_space(l, 1, LV_PART_MAIN);
  lv_obj_center(l);
  return b;
}

lv_obj_t *draw_cell(int row, int col, int w_cells, const char *text, uint32_t accent)
{
  return draw_cell_impl(row, col, w_cells, 1, text, accent, 80, false);
}

lv_obj_t *draw_cell_ext(int row, int col, int w_cells, int h_cells, const char *text, uint32_t accent, bool filled)
{
  return draw_cell_impl(row, col, w_cells, h_cells, text, accent, 80, filled);
}

/* Icon-faced button: `icon` is a weak symbol from src/icons/ (see its README) --
 * NULL when the asset file is absent, in which case the text fallback draws
 * instead. Icons are recolored to the accent, so draw them white-on-transparent. */
lv_obj_t *draw_cell_icon_ext(int row, int col, int w_cells, int h_cells, const lv_image_dsc_t *icon, const char *fallback,
                        uint32_t accent)
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
  lv_obj_set_style_image_recolor(img, lv_color_hex(accent), LV_PART_MAIN);
  lv_obj_set_style_image_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_center(img);
  return b;
}

lv_obj_t *draw_cell_icon(int row, int col, const lv_image_dsc_t *icon, const char *fallback,
                    uint32_t accent)
{
  return draw_cell_icon_ext(row, col, 1, 1, icon, fallback, accent);
}
