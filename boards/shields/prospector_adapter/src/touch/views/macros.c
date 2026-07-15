/* --------------------------------- MACROS ---------------------------------- */

#include "../touch_ui.h"

static const struct page_cell pad_cells[];
static const struct page_cell pad_cells_portrait[];

static void build_pad(void)
{
  int n = touch_pad_count();
  const struct page_cell *active = (ui_rot & 1) ? pad_cells_portrait : pad_cells;
  for (int i = 0; active[i].row_span != 0 || active[i].col_span != 0; i++)
  {
    if (active[i].action == ACT_CUSTOM_VAL && cur_view_btns[i])
    {
      int pad_idx = active[i].arg.custom.val;
      lv_obj_set_style_border_color(cur_view_btns[i], lv_color_hex(theme_color(pad_idx < n ? THEME_PRIMARY : THEME_MUTED)), LV_PART_MAIN);
      cell_child_set_color(cur_view_btns[i], pad_idx < n ? THEME_PRIMARY : THEME_MUTED);
    }
  }
}

static const struct page_cell pad_cells[] = {
    {0, 0, 1, 1, NULL, &icon_terminal, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 0}},
    {0, 1, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 2, 1, 1, NULL, &icon_list, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 1}},
    {1, 0, 1, 1, NULL, &icon_browser, THEME_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 2}},
    {1, 1, 1, 1, NULL, &icon_desktop, THEME_FOCUS, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 3}},
    {1, 2, 1, 1, NULL, &icon_notes, THEME_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 4}},
    {0}};

static const struct page_cell pad_cells_portrait[] = {
    {0, 0, 1, 1, NULL, &icon_terminal, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 0}},
    {0, 1, 1, 1, NULL, &icon_list, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 1}},
    {1, 0, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {1, 1, 1, 1, NULL, &icon_desktop, THEME_FOCUS, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 3}},
    {2, 0, 1, 1, NULL, &icon_browser, THEME_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 2}},
    {2, 1, 1, 1, NULL, &icon_notes, THEME_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {fire_pad, 4}},
    {0}};

const struct view_def view_pad = {
    .cells = pad_cells,
    .cells_portrait = pad_cells_portrait,
    .build = build_pad,
};
