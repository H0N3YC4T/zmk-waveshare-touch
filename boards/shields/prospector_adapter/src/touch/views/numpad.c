/* -------------------------------- NUMPAD ----------------------------------- */

#include "../touch_ui.h"

static const struct page_cell numpad_cells[] = {
    {0, 0, 1, 1, "7", NULL, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = KP_N7},
    {0, 1, 1, 1, "8", NULL, THEME_SECONDARY, ACT_SEND_KEY, .arg.keycode = KP_N8},
    {0, 2, 1, 1, "9", NULL, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = KP_N9},
    {0, 3, 1, 1, "+", NULL, THEME_FOCUS, ACT_SEND_KEY, .arg.keycode = KP_PLUS},

    {1, 0, 1, 1, "4", NULL, THEME_SECONDARY, ACT_SEND_KEY, .arg.keycode = KP_N4},
    {1, 1, 1, 1, "5", NULL, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = KP_N5},
    {1, 2, 1, 1, "6", NULL, THEME_SECONDARY, ACT_SEND_KEY, .arg.keycode = KP_N6},
    {1, 3, 1, 1, "-", NULL, THEME_FOCUS, ACT_SEND_KEY, .arg.keycode = KP_MINUS},

    {2, 0, 1, 1, "1", NULL, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = KP_N1},
    {2, 1, 1, 1, "2", NULL, THEME_SECONDARY, ACT_SEND_KEY, .arg.keycode = KP_N2},
    {2, 2, 1, 1, "3", NULL, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = KP_N3},
    {2, 3, 1, 1, "*", NULL, THEME_FOCUS, ACT_SEND_KEY, .arg.keycode = KP_MULTIPLY},

    {3, 0, 1, 1, NULL, &icon_up_32, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {3, 1, 1, 1, "0", NULL, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = KP_N0},
    {3, 2, 1, 1, NULL, &icon_newline_32, THEME_ACCEPT, ACT_SEND_KEY, .arg.keycode = KP_ENTER},
    {3, 3, 1, 1, "/", NULL, THEME_FOCUS, ACT_SEND_KEY, .arg.keycode = KP_DIVIDE},

    {0}};

const struct view_def view_numpad = {
    .cells = numpad_cells,
    .keeps_mods = true,
};
