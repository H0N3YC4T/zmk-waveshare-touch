
#include <zephyr/device.h>
#include "touch_ui.h"

/* ----------------------------- the registry -------------------------------- */

static const struct page_cell *get_active_cells(const struct view_def *d)
{
  if (!d) return NULL;
  const struct page_cell *active_cells = d->cells;
  /* pages[] holds pages 2..num_pages, so its last valid index is num_pages - 2 */
  bool paged = cur_page > 0 && (cur_page - 1) < d->num_pages - 1;
  if (paged && d->pages != NULL) {
      active_cells = d->pages[cur_page - 1];
  }
  if (ui_rot & 1) {
      if (d->cells_portrait != NULL) {
          active_cells = d->cells_portrait;
      }
      if (paged && d->pages_portrait != NULL) {
          active_cells = d->pages_portrait[cur_page - 1];
      }
  }
  return active_cells;
}

/* Span-aware hit test: the current view's cell covering grid cell `cell`. */
static const struct page_cell *find_cell_at(int cell)
{
  const struct page_cell *active_cells = get_active_cells(cur_view);
  if (!active_cells || grid_cols <= 0) return NULL;

  int r = cell / grid_cols;
  int c = cell % grid_cols;

  for (int i = 0; active_cells[i].row_span != 0 || active_cells[i].col_span != 0; i++) {
    const struct page_cell *pc = &active_cells[i];
    int rs = pc->row_span > 0 ? pc->row_span : 1;
    int cs = pc->col_span > 0 ? pc->col_span : 1;
    if (r >= pc->row && r < pc->row + rs && c >= pc->col && c < pc->col + cs) {
      return pc;
    }
  }
  return NULL;
}

bool ui_has_action(int cell)
{
  const struct page_cell *pc = find_cell_at(cell);
  return pc != NULL && (pc->action != ACT_NONE || pc->label != NULL || pc->icon != NULL);
}

void tap_declarative(int cell)
{
  const struct view_def *d = cur_view;
  const struct page_cell *pc = find_cell_at(cell);
  if (pc != NULL) {
      switch (pc->action) {
        case ACT_GO_VIEW:
          show_view(pc->arg.view);
          break;
        case ACT_SEND_KEY:
          send_key(pc->arg.keycode);
          break;
        case ACT_FIRE_MACRO:
          fire_macro(pc->arg.macro);
          break;
        case ACT_CUSTOM_VAL:
          if (pc->arg.custom.cb) pc->arg.custom.cb(pc->arg.custom.val);
          break;
        case ACT_CUSTOM:
          if (pc->arg.func) pc->arg.func(cell);
          break;
        case ACT_NEXT_PAGE:
          if (d->num_pages > 0) {
              cur_page = (cur_page + 1) % d->num_pages;
              build_view(cur_view);
          }
          break;
        case ACT_PREV_PAGE:
          if (cur_page == 0) {
              show_view(&view_home);
          } else {
              cur_page--;
              build_view(cur_view);
          }
          break;
        default:
          break;
      }
  }
}

/* ----------------------------- dispatcher ---------------------------------- */

lv_obj_t *cur_view_btns[32];

void build_view(const struct view_def *d)
{
  lv_obj_clean(touch_overlay);
  memset(cur_view_btns, 0, sizeof(cur_view_btns));

  const struct page_cell *active_cells = get_active_cells(d);

  if (active_cells != NULL) {
    int max_r = 1, max_c = 1;
    for (int i = 0; active_cells[i].row_span != 0 || active_cells[i].col_span != 0; i++) {
        int r = active_cells[i].row + (active_cells[i].row_span > 0 ? active_cells[i].row_span : 1);
        int c = active_cells[i].col + (active_cells[i].col_span > 0 ? active_cells[i].col_span : 1);
        if (r > max_r) max_r = r;
        if (c > max_c) max_c = c;
    }
    grid_rows = max_r;
    grid_cols = max_c;

    for (int i = 0; active_cells[i].row_span != 0 || active_cells[i].col_span != 0; i++) {
        const struct page_cell *pc = &active_cells[i];
        int rs = pc->row_span > 0 ? pc->row_span : 1;
        int cs = pc->col_span > 0 ? pc->col_span : 1;
        if (i < 32) {
            if (pc->icon) {
                cur_view_btns[i] = draw_cell_icon_ext(pc->row, pc->col, cs, rs, pc->icon, pc->label, pc->color);
            } else if (pc->label) {
                cur_view_btns[i] = draw_cell_ext(pc->row, pc->col, cs, rs, pc->label, pc->color, false);
            }
        }
    }
  }

  if (d->build != NULL)
  {
    d->build();
  }


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

