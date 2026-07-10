/* The registry of all touch UI views and the build_view() dispatcher.
 * Per-view renderers (build_*) and tap handlers (tap_*) live in views/ --
 * one file per view -- and are declared in touch_ui.h. Navigation
 * (touch_nav.c) only ever consults the registry below. */

#include <zephyr/device.h>
#include "touch_ui.h"

/* ----------------------------- the registry -------------------------------- */

static const struct page_cell *get_active_cells(const struct view_def *d)
{
  if (!d) return NULL;
  const struct page_cell *active_cells = d->cells;
  if (cur_page > 0 && d->pages != NULL && (cur_page - 1) < d->num_pages) {
      active_cells = d->pages[cur_page - 1];
  }
  if (ui_rot & 1) {
      if (d->cells_portrait != NULL) {
          active_cells = d->cells_portrait;
      }
      if (cur_page > 0 && d->pages_portrait != NULL && (cur_page - 1) < d->num_pages) {
          active_cells = d->pages_portrait[cur_page - 1];
      }
  }
  return active_cells;
}

bool ui_has_action(int cell)
{
  const struct view_def *d = cur_view;
  if (!d) return false;
  
  const struct page_cell *active_cells = get_active_cells(d);

  if (!active_cells || grid_cols <= 0) return false;

  int r = cell / grid_cols;
  int c = cell % grid_cols;

  for (int i = 0; active_cells[i].row_span != 0 || active_cells[i].col_span != 0; i++) {
    const struct page_cell *pc = &active_cells[i];
    int rs = pc->row_span > 0 ? pc->row_span : 1;
    int cs = pc->col_span > 0 ? pc->col_span : 1;

    if (r >= pc->row && r < pc->row + rs && c >= pc->col && c < pc->col + cs) {
      if (pc->action != ACT_NONE || pc->label != NULL || pc->icon != NULL) {
          return true;
      }
      return false;
    }
  }
  return false;
}

void tap_declarative(int cell)
{
  const struct view_def *d = cur_view;
  const struct page_cell *active_cells = get_active_cells(d);

  if (!active_cells) return;

  int r = cell / grid_cols;
  int c = cell % grid_cols;

  for (int i = 0; active_cells[i].row_span != 0 || active_cells[i].col_span != 0; i++) {
    const struct page_cell *pc = &active_cells[i];
    int rs = pc->row_span > 0 ? pc->row_span : 1;
    int cs = pc->col_span > 0 ? pc->col_span : 1;

    if (r >= pc->row && r < pc->row + rs && c >= pc->col && c < pc->col + cs) {
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
      return;
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

