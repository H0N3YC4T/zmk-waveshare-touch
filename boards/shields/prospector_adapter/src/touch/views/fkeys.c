/* --------------------------------- FKEYS ----------------------------------- */


#include "../touch_ui.h"

static const struct page_cell fkeys_p0[] = {
    {0, 0, 1, 1, "F1", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F1},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_RED, ACT_PREV_PAGE},
    {0, 2, 1, 1, "F2", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F2},
    {1, 0, 1, 1, "F3", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F3},
    {1, 1, 1, 1, "F4", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F4},
    {1, 2, 1, 1, "F5", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F5},
    {2, 0, 1, 1, "F6", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F6},
    {2, 1, 1, 1, LV_SYMBOL_DOWN, NULL, COLOR_ALERT, ACT_NEXT_PAGE},
    {2, 2, 1, 1, "F7", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F7},
    {0}
};

static const struct page_cell fkeys_p1[] = {
    {0, 0, 1, 1, "F8", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F8},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_ALERT, ACT_PREV_PAGE},
    {0, 2, 1, 1, "F9", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F9},
    {1, 0, 1, 1, "F10", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F10},
    {1, 1, 1, 1, "F11", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F11},
    {1, 2, 1, 1, "F12", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = F12},
    {0}
};

static const struct page_cell *const fkeys_pages[] = { fkeys_p1 };

const struct view_def view_fkeys = {
    .cells = fkeys_p0,
    .pages = fkeys_pages,
    .num_pages = 2,
    .keeps_mods = true,
};
