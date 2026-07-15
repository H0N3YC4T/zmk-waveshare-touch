/* -------------------------------- MEDIA ------------------------------------ */

#include "../touch_ui.h"

static const struct page_cell media_cells[] = {
    {0, 0, 1, 1, NULL, &icon_voldown, THEME_DECREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_0"},
    {0, 1, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 2, 1, 1, NULL, &icon_volup, THEME_INCREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_2"},
    {1, 0, 1, 1, NULL, &icon_prev, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_4"},
    {1, 1, 1, 1, NULL, &icon_play, THEME_SECONDARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_3"},
    {1, 2, 1, 1, NULL, &icon_next, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_5"},
    {0}};

static const struct page_cell media_cells_portrait[] = {
    {0, 0, 1, 1, NULL, &icon_voldown, THEME_DECREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_0"},
    {0, 1, 1, 1, NULL, &icon_volup, THEME_INCREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_2"},
    {1, 0, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {1, 1, 1, 1, NULL, &icon_play, THEME_SECONDARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_3"},
    {2, 0, 1, 1, NULL, &icon_prev, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_4"},
    {2, 1, 1, 1, NULL, &icon_next, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_5"},
    {0}};

const struct view_def view_media = {
    .cells = media_cells,
    .cells_portrait = media_cells_portrait,
};
