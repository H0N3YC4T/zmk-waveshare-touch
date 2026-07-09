/* --------------------------- FKEYS / SYMBOLS ------------------------------- */
/* Paginated 3x3 key screens: 7 keys per page; cell 1 = Back (page 0) / Prev,
 * cell 7 = Next. Key sends go through send_key (applies any armed one-shot mod). */

#include "../touch_ui.h"

static const uint32_t fkeys[12] = {F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12};
static const char *const fkey_lbls[12] = {"F1", "F2", "F3", "F4", "F5", "F6",
                                          "F7", "F8", "F9", "F10", "F11", "F12"};

static const uint32_t symbols[32] = {
    EXCL,
    AT,
    HASH,
    DLLR,
    PRCNT,
    CARET,
    AMPS,
    STAR,
    LPAR,
    RPAR,
    MINUS,
    UNDER,
    EQUAL,
    PLUS,
    LBKT,
    RBKT,
    LBRC,
    RBRC,
    BSLH,
    PIPE,
    SEMI,
    COLON,
    SQT,
    DQT,
    COMMA,
    DOT,
    LT,
    GT,
    FSLH,
    QMARK,
    GRAVE,
    TILDE,
};
static const char *const sym_lbls[32] = {
    "!",
    "@",
    "#",
    "$",
    "%",
    "^",
    "&",
    "*",
    "(",
    ")",
    "-",
    "_",
    "=",
    "+",
    "[",
    "]",
    "{",
    "}",
    "\\",
    "|",
    ";",
    ":",
    "'",
    "\"",
    ",",
    ".",
    "<",
    ">",
    "/",
    "?",
    "`",
    "~",
};

static void handle_key_page(const uint32_t *keys, int n, int cell)
{
  int pages = (n + KEYS_PER_PAGE - 1) / KEYS_PER_PAGE;
  if (cell == 1)
  { /* Back (page 0) or Prev */
    if (cur_page == 0)
    {
      show_view(VIEW_HOME);
    }
    else
    {
      cur_page--;
      build_view(cur_view);
    }
    return;
  }
  if (cell == 7)
  { /* Next (cyclic) */
    if (pages > 1)
    {
      cur_page = (cur_page + 1) % pages;
      build_view(cur_view);
    }
    return;
  }
  for (int i = 0; i < KEYS_PER_PAGE; i++)
  {
    if (key_cells[i] == cell)
    {
      int idx = cur_page * KEYS_PER_PAGE + i;
      if (idx < n)
      {
        send_key(keys[idx]);
      }
      return;
    }
  }
}

void build_fkeys(void) { draw_key_page(fkey_lbls, 12, cur_page); }
void tap_fkeys(int cell) { handle_key_page(fkeys, 12, cell); }
void build_symbols(void) { draw_key_page(sym_lbls, 32, cur_page); }
void tap_symbols(int cell) { handle_key_page(symbols, 32, cell); }
