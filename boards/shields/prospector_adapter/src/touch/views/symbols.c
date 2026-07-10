/* -------------------------------- SYMBOLS ---------------------------------- */


#include "../touch_ui.h"

static const struct page_cell sym_p0[] = {
    {0, 0, 1, 1, "!", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = EXCL},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_RED, ACT_PREV_PAGE},
    {0, 2, 1, 1, "@", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = AT},
    {1, 0, 1, 1, "#", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = HASH},
    {1, 1, 1, 1, "$", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = DLLR},
    {1, 2, 1, 1, "%", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = PRCNT},
    {2, 0, 1, 1, "^", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = CARET},
    {2, 1, 1, 1, LV_SYMBOL_DOWN, NULL, COLOR_ALERT, ACT_NEXT_PAGE},
    {2, 2, 1, 1, "&", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = AMPS},
    {0}
};

static const struct page_cell sym_p1[] = {
    {0, 0, 1, 1, "*", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = STAR},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_ALERT, ACT_PREV_PAGE},
    {0, 2, 1, 1, "(", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = LPAR},
    {1, 0, 1, 1, ")", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = RPAR},
    {1, 1, 1, 1, "-", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = MINUS},
    {1, 2, 1, 1, "_", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = UNDER},
    {2, 0, 1, 1, "=", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = EQUAL},
    {2, 1, 1, 1, LV_SYMBOL_DOWN, NULL, COLOR_ALERT, ACT_NEXT_PAGE},
    {2, 2, 1, 1, "+", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = PLUS},
    {0}
};

static const struct page_cell sym_p2[] = {
    {0, 0, 1, 1, "[", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = LBKT},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_ALERT, ACT_PREV_PAGE},
    {0, 2, 1, 1, "]", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = RBKT},
    {1, 0, 1, 1, "{", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = LBRC},
    {1, 1, 1, 1, "}", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = RBRC},
    {1, 2, 1, 1, "\\", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = BSLH},
    {2, 0, 1, 1, "|", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = PIPE},
    {2, 1, 1, 1, LV_SYMBOL_DOWN, NULL, COLOR_ALERT, ACT_NEXT_PAGE},
    {2, 2, 1, 1, ";", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = SEMI},
    {0}
};

static const struct page_cell sym_p3[] = {
    {0, 0, 1, 1, ":", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = COLON},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_ALERT, ACT_PREV_PAGE},
    {0, 2, 1, 1, "'", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = SQT},
    {1, 0, 1, 1, "\"", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = DQT},
    {1, 1, 1, 1, ",", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = COMMA},
    {1, 2, 1, 1, ".", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = DOT},
    {2, 0, 1, 1, "<", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = LT},
    {2, 1, 1, 1, LV_SYMBOL_DOWN, NULL, COLOR_ALERT, ACT_NEXT_PAGE},
    {2, 2, 1, 1, ">", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = GT},
    {0}
};

static const struct page_cell sym_p4[] = {
    {0, 0, 1, 1, "/", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = FSLH},
    {0, 1, 1, 1, LV_SYMBOL_UP, NULL, COLOR_ALERT, ACT_PREV_PAGE},
    {0, 2, 1, 1, "?", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = QMARK},
    {1, 0, 1, 1, "`", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = GRAVE},
    {1, 1, 1, 1, "~", NULL, COLOR_PRIMARY, ACT_SEND_KEY, .arg.keycode = TILDE},
    {0}
};

static const struct page_cell *const sym_pages[] = { sym_p1, sym_p2, sym_p3, sym_p4 };

const struct view_def view_symbols = {
    .cells = sym_p0,
    .pages = sym_pages,
    .num_pages = 5,
    .keeps_mods = true,
};
