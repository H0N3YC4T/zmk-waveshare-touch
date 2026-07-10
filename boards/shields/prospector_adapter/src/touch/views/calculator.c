#include <stdio.h>
#include <string.h>
#include "../touch_ui.h"

/* ========================================================================== */
/* STATE                                                                      */
/* ========================================================================== */
static char calc_expr[64];
static const char *calc_cp;
static bool calc_shown;
static bool eval_okay;
static bool bracket_open;

static bool bin_mode;
static bool bin_converted;
static int bin_word_size = 8;

/* ========================================================================== */
/* FORWARD DECLARATIONS (Required for internal cyclic calls)                  */
/* ========================================================================== */
static double parse_expr(void);
static void calc_print_result(double result);
static void calc_update_display(void);

/* ========================================================================== */
/* MATH ENGINE & PARSER                                                       */
/* ========================================================================== */
static double parse_factor(void) {
  double sum = 0;
  bool num_empty = true;
  bool is_negative = false;
  bool is_bitwise_not = false;

  if (*calc_cp == '~') {
    is_bitwise_not = true;
    calc_cp++;
  }

  if (*calc_cp == '-') {
    is_negative = true;
    calc_cp++;
  } else if (*calc_cp == '+') {
    calc_cp++;
  }

  if (*calc_cp == '(') {
    calc_cp++;
    sum = parse_expr();
    if (*calc_cp == ')') calc_cp++;
    else eval_okay = false;
    sum = is_negative ? -sum : sum;
    if (is_bitwise_not) sum = (double)(~(long long)sum);
    return sum;
  }

  if (bin_mode) {
    while (*calc_cp == '0' || *calc_cp == '1') {
      sum = sum * 2 + (*calc_cp - '0');
      calc_cp++;
      num_empty = false;
    }
  } else {
    while (*calc_cp >= '0' && *calc_cp <= '9') {
      sum = sum * 10 + (*calc_cp - '0');
      calc_cp++;
      num_empty = false;
    }
    if (*calc_cp == '.') {
      calc_cp++;
      double fraction = 0.1;
      while (*calc_cp >= '0' && *calc_cp <= '9') {
        sum += (*calc_cp - '0') * fraction;
        fraction /= 10.0;
        calc_cp++;
        num_empty = false;
      }
    }
  }

  if (*calc_cp == '!') {
    calc_cp++;
    if (sum >= 0 && sum == (long long)sum) {
      long long n = (long long)sum;
      if (n > 20) eval_okay = false;
      else {
        double f = 1;
        for (long long i = 2; i <= n; i++) f *= i;
        sum = f;
      }
    } else {
      eval_okay = false;
    }
  }

  if (num_empty) eval_okay = false;
  sum = is_negative ? -sum : sum;
  if (is_bitwise_not) sum = (double)(~(long long)sum);
  return sum;
}

static double parse_term(void) {
  double a = parse_factor();
  while (eval_okay && (*calc_cp == '*' || *calc_cp == '/' || *calc_cp == '%')) {
    char op = *calc_cp++;
    double b = parse_factor();
    if (op == '*') a *= b;
    else if (op == '%') {
      if (b == 0) { eval_okay = false; a = 0; }
      else a = (double)((long long)a % (long long)b);
    } else if (b == 0) {
      eval_okay = false; a = 0;
    } else a /= b;
  }
  return a;
}

static double parse_add_sub(void) {
  double a = parse_term();
  while (eval_okay && (*calc_cp == '+' || *calc_cp == '-')) {
    char op = *calc_cp++;
    double b = parse_term();
    a = (op == '+') ? a + b : a - b;
  }
  return a;
}

static double parse_shift(void) {
  double a = parse_add_sub();
  while (eval_okay && ((*calc_cp == '<' && *(calc_cp + 1) == '<') ||
                       (*calc_cp == '>' && *(calc_cp + 1) == '>'))) {
    char op = *calc_cp;
    calc_cp += 2;
    double b = parse_add_sub();
    long long shift_amt = (long long)b;
    if (shift_amt < 0 || shift_amt > 63) eval_okay = false;
    else {
      if (op == '<') a = (double)((long long)a << shift_amt);
      else a = (double)((long long)a >> shift_amt);
    }
  }
  return a;
}

static double parse_bitwise_and(void) {
  double a = parse_shift();
  while (eval_okay && (*calc_cp == '&')) {
    calc_cp++;
    double b = parse_shift();
    a = (double)((long long)a & (long long)b);
  }
  return a;
}

static double parse_bitwise_xor(void) {
  double a = parse_bitwise_and();
  while (eval_okay && (*calc_cp == '^')) {
    calc_cp++;
    double b = parse_bitwise_and();
    a = (double)((long long)a ^ (long long)b);
  }
  return a;
}

static double parse_bitwise_or(void) {
  double a = parse_bitwise_xor();
  while (eval_okay && (*calc_cp == '|')) {
    calc_cp++;
    double b = parse_bitwise_xor();
    a = (double)((long long)a | (long long)b);
  }
  return a;
}

static double parse_expr(void) { return parse_bitwise_or(); }

/* Parse the whole expression. Anything the grammar didn't consume (e.g. a lone
 * '<' from a half-deleted shift) flags an error instead of silently returning
 * the partial result. */
static double calc_run_parser(void) {
  eval_okay = true;
  calc_cp = &calc_expr[0];
  double r = (*calc_cp != '\0') ? parse_expr() : 0;
  if (*calc_cp != '\0') eval_okay = false;
  return r;
}

/* ========================================================================== */
/* USER INPUT & STATE UPDATES                                                 */
/* ========================================================================== */
static void calc_push(char ch) {
  if (calc_shown) {
    if ((ch >= '0' && ch <= '9') || ch == '.') {
      calc_expr[0] = '\0';
    }
    calc_shown = false;
    bracket_open = false;
  }
  size_t n = strlen(calc_expr);
  if (n < sizeof(calc_expr) - 1) {
    calc_expr[n] = ch;
    calc_expr[n + 1] = '\0';
  }
}

static void calc_tap_backspace(void) {
  if (calc_shown) {
    calc_expr[0] = '\0';
    calc_shown = false;
    bracket_open = false;
  } else {
    size_t n = strlen(calc_expr);
    if (n) {
      char deleted = calc_expr[n - 1];
      calc_expr[n - 1] = '\0';
      if (deleted == '(') bracket_open = false;
      else if (deleted == ')') bracket_open = true;
      /* shifts are two chars ('<<' / '>>') -- delete them as one keystroke */
      else if ((deleted == '<' || deleted == '>') && n >= 2 &&
               calc_expr[n - 2] == deleted) {
        calc_expr[n - 2] = '\0';
      }
    }
  }
}

static void calc_print_result(double result) {
  if (bin_mode) {
    long long val = (long long)(result > 0 ? result + 0.5 : result - 0.5);
    unsigned long long mask = bin_word_size == 64
                                  ? 0xFFFFFFFFFFFFFFFFULL
                                  : (1ULL << bin_word_size) - 1;
    val = val & mask;
    if (val == 0) {
      snprintf(calc_expr, sizeof(calc_expr), "0");
    } else {
      char buf[65];
      int i = 0;
      unsigned long long u = val;
      while (u > 0 && i < 64) {
        buf[i++] = (u % 2) ? '1' : '0';
        u /= 2;
      }
      buf[i] = '\0';
      for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
      }
      snprintf(calc_expr, sizeof(calc_expr), "%s", buf);
    }
    return;
  }

  int is_negative = result < 0;
  result = is_negative ? -result : result;

  long long whole = (long long)result;
  int fraction = (int)((result - whole) * 100.0 + 0.5);
  if (fraction >= 100) {
    whole++;
    fraction = 0;
  }

  snprintf(calc_expr, sizeof(calc_expr), "%s%lld.%02d",
           (is_negative && (whole || fraction)) ? "-" : "", whole, fraction);

  int len = strlen(calc_expr);
  while (len > 0 && calc_expr[len - 1] == '0')
    calc_expr[--len] = '\0';
  if (len > 0 && calc_expr[len - 1] == '.')
    calc_expr[--len] = '\0';
}

static void calc_tap_eval(void) {
  if (bracket_open) calc_push(')');
  double result = calc_run_parser();
  if (eval_okay) calc_print_result(result);
  else snprintf(calc_expr, sizeof(calc_expr), "Error");
  calc_shown = true;
  bracket_open = false;
}

/* ========================================================================== */
/* CONVERSION HELPERS                                                         */
/* ========================================================================== */
static void convert_to_binary(void) {
    if (calc_expr[0] == '\0') return;

    bool old_mode = bin_mode;
    bin_mode = false;
    double r = calc_run_parser();
    bin_mode = old_mode;

    if (eval_okay) {
        long long val = (long long)(r > 0 ? r + 0.5 : r - 0.5);

        unsigned long long abs_val = val >= 0 ? val : -val;
        if (abs_val <= 3) bin_word_size = 2;
        else if (abs_val <= 15) bin_word_size = 4;
        else if (abs_val <= 255) bin_word_size = 8;
        else if (abs_val <= 65535) bin_word_size = 16;
        else { val = 0; bin_word_size = 16; }

        bin_mode = true;
        calc_print_result(val);
        bin_converted = true;
        calc_shown = true;
        bracket_open = false;
    } else {
        snprintf(calc_expr, sizeof(calc_expr), "Error");
        calc_shown = true;
    }
}

static void convert_to_decimal(void) {
    if (calc_expr[0] == '\0') return;

    bool old_mode = bin_mode;
    bin_mode = true;
    double r = calc_run_parser();
    bin_mode = old_mode;

    if (eval_okay) {
        bin_mode = false;
        calc_print_result(r);
        bin_converted = false;
        calc_shown = true;
        bracket_open = false;
    } else {
        snprintf(calc_expr, sizeof(calc_expr), "Error");
        calc_shown = true;
    }
}

static void calc_ensure_binary(void) {
  if (bin_mode && !bin_converted) {
    convert_to_binary();
  }
}

/* ========================================================================== */
/* UI & LIFECYCLE HANDLERS                                                    */
/* ========================================================================== */
static void calc_on_enter(void) {
  calc_expr[0] = '\0';
  calc_shown = false;
  bracket_open = false;
  bin_mode = false;
  bin_converted = false;
}

static void calc_update_display(void) {
  if (cur_view_btns[0]) {
    lv_obj_t *l = lv_obj_get_child(cur_view_btns[0], 0);
    if (l) {
      lv_label_set_text(l, calc_expr[0] ? calc_expr : "0");
    }
  }
  if (cur_page == 2 && cur_view_btns[14]) {
    lv_obj_t *ws_label = lv_obj_get_child(cur_view_btns[14], 0);
    if (ws_label) {
      static char ws_text[8];
      snprintf(ws_text, sizeof(ws_text), "%db", bin_word_size);
      lv_label_set_text(ws_label, ws_text);
    }
  }
}

/* ========================================================================== */
/* TAP HANDLERS (DECIMAL)                                                     */
/* ========================================================================== */
static void tap_calc_char(int ch) {
  calc_ensure_binary();
  calc_push((char)ch);
  calc_update_display();
}

/* Shift buttons: the parser reads << / >> as two chars, so one tap pushes both. */
static void tap_calc_shift(int ch) {
  calc_ensure_binary();
  calc_push((char)ch);
  calc_push((char)ch);
  calc_update_display();
}

static void tap_calc_14(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_tap_eval(); calc_update_display(); }

static void tap_calc_backspace_cb(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_tap_backspace();
  calc_update_display();
}
static void tap_calc_dot(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('.');
  calc_update_display();
}
static void tap_calc_bracket(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  if (!bracket_open) {
    size_t n = strlen(calc_expr);
    if (n > 0 && ((calc_expr[n - 1] >= '0' && calc_expr[n - 1] <= '9') ||
                  calc_expr[n - 1] == ')')) {
      calc_push('*');
    }
    calc_push('(');
    bracket_open = true;
  } else {
    calc_push(')');
    bracket_open = false;
  }
  calc_update_display();
}
static void tap_calc_percent(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('%');
  calc_update_display();
}
static void tap_calc_factorial(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('!');
  calc_update_display();
}

static void hold_calc(int cell) {
  if ((cur_page < 2 && cell == 3) || (cur_page == 2 && cell == 18)) {
    calc_expr[0] = '\0';
    calc_shown = false;
    bracket_open = false;
    calc_update_display();
  } else if ((cur_page == 0 && cell == 19) || (cur_page == 2 && cell == 7)) {
    tap_calc_percent(cell);
  } else {
    tap_declarative(cell);
  }
}

/* ========================================================================== */
/* TAP HANDLERS (BINARY)                                                      */
/* ========================================================================== */
static void tap_calc_ws(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  if (bin_word_size == 16)
    bin_word_size = 8;
  else if (bin_word_size == 8)
    bin_word_size = 4;
  else if (bin_word_size == 4)
    bin_word_size = 2;
  else
    bin_word_size = 16;
  bin_converted = false;

  if (calc_expr[0] != '\0') {
    double r = calc_run_parser();
    if (eval_okay) {
      long long val = (long long)(r > 0 ? r + 0.5 : r - 0.5);
      unsigned long long mask = bin_word_size == 64
                                    ? 0xFFFFFFFFFFFFFFFFULL
                                    : (1ULL << bin_word_size) - 1;
      val = val & mask;
      if (val == 0)
        snprintf(calc_expr, sizeof(calc_expr), "0");
      else {
        char buf[65];
        int i = 0;
        unsigned long long u = val;
        while (u > 0 && i < 64) {
          buf[i++] = (u % 2) ? '1' : '0';
          u /= 2;
        }
        buf[i] = '\0';
        for (int j = 0; j < i / 2; j++) {
          char tmp = buf[j];
          buf[j] = buf[i - 1 - j];
          buf[i - 1 - j] = tmp;
        }
        snprintf(calc_expr, sizeof(calc_expr), "%s", buf);
      }
    } else {
      snprintf(calc_expr, sizeof(calc_expr), "Error");
    }
  }
  calc_shown = true;
  bracket_open = false;
  calc_update_display();
}


/* ========================================================================== */
/* CELL OBJECTS & LAYOUTS                                                     */
/* ========================================================================== */

static void tap_calc_to_bin(int cell);
static void tap_calc_to_dec(int cell);
static void tap_calc_to_page2(int cell);

static const struct page_cell calc_cells[] = {
    {0, 0, 1, 3, "0", NULL, COLOR_ACCENT, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 3, 1, 1, LV_SYMBOL_BACKSPACE, NULL, COLOR_RED, ACT_CUSTOM, .arg.func = tap_calc_backspace_cb},

    {1, 0, 1, 1, "7", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '7'}},
    {1, 1, 1, 1, "8", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '8'}},
    {1, 2, 1, 1, "9", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '9'}},
    {1, 3, 1, 1, "+", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '+'}},

    {2, 0, 1, 1, "4", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '4'}},
    {2, 1, 1, 1, "5", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '5'}},
    {2, 2, 1, 1, "6", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '6'}},
    {2, 3, 1, 1, "-", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '-'}},

    {3, 0, 1, 1, "1", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '1'}},
    {3, 1, 1, 1, "2", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '2'}},
    {3, 2, 1, 1, "3", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '3'}},
    {3, 3, 1, 1, "*", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '*'}},

    {4, 0, 1, 1, LV_SYMBOL_NEXT, NULL, COLOR_ALERT, ACT_CUSTOM, .arg.func = tap_calc_to_page2},
    {4, 1, 1, 1, "0", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '0'}},
    {4, 2, 1, 1, "=", NULL, COLOR_GREEN, ACT_CUSTOM, .arg.func = tap_calc_14},
    {4, 3, 1, 1, "/", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '/'}},
    {0}};

static const struct page_cell calc_cells_alt[] = {
    {0, 0, 1, 3, "0", NULL, COLOR_ACCENT, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 3, 1, 1, LV_SYMBOL_BACKSPACE, NULL, COLOR_RED, ACT_CUSTOM, .arg.func = tap_calc_backspace_cb},

    {1, 0, 1, 1, "7", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '7'}},
    {1, 1, 1, 1, "8", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '8'}},
    {1, 2, 1, 1, "9", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '9'}},
    {1, 3, 1, 1, "()", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_bracket},

    {2, 0, 1, 1, "4", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '4'}},
    {2, 1, 1, 1, "5", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '5'}},
    {2, 2, 1, 1, "6", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '6'}},
    {2, 3, 1, 1, "%", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_percent},

    {3, 0, 1, 1, "1", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '1'}},
    {3, 1, 1, 1, "2", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '2'}},
    {3, 2, 1, 1, "3", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '3'}},
    {3, 3, 1, 1, "!", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_factorial},

    {4, 0, 1, 1, LV_SYMBOL_PREV, NULL, COLOR_ALERT, ACT_CUSTOM, .arg.func = tap_calc_to_dec},
    {4, 1, 1, 1, "0", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '0'}},
    {4, 2, 1, 1, LV_SYMBOL_NEXT, NULL, COLOR_ALERT, ACT_CUSTOM, .arg.func = tap_calc_to_bin},
    {4, 3, 1, 1, ".", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_dot},
    {0}};

static const struct page_cell calc_cells_alt2[] = {
    {0, 0, 1, 4, "0", NULL, COLOR_ACCENT, ACT_GO_VIEW, .arg.view = &view_home},

    {1, 0, 1, 1, "+", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '+'}},
    {1, 1, 1, 1, "-", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '-'}},
    {1, 2, 1, 1, "*", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '*'}},
    {1, 3, 1, 1, "/", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '/'}},

    {2, 0, 1, 1, "<<", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_shift, '<'}},
    {2, 1, 1, 1, "0", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '0'}},
    {2, 2, 1, 1, "1", NULL, COLOR_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '1'}},
    {2, 3, 1, 1, ">>", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_shift, '>'}},

    {3, 0, 1, 1, "&", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '&'}},
    {3, 1, 1, 1, "|", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '|'}},
    {3, 2, 1, 1, "~", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '~'}},
    {3, 3, 1, 1, "^", NULL, COLOR_ACCENT, ACT_CUSTOM_VAL, .arg.custom = {tap_calc_char, '^'}},

    {4, 0, 1, 1, LV_SYMBOL_PREV, NULL, COLOR_ALERT, ACT_CUSTOM, .arg.func = tap_calc_to_dec},
    {4, 1, 1, 1, "WS", NULL, COLOR_ALERT, ACT_CUSTOM, .arg.func = tap_calc_ws},
    {4, 2, 1, 1, LV_SYMBOL_BACKSPACE, NULL, COLOR_RED, ACT_CUSTOM, .arg.func = tap_calc_backspace_cb},
    {4, 3, 1, 1, "=", NULL, COLOR_GREEN, ACT_CUSTOM, .arg.func = tap_calc_14},
    {0}};

static const struct page_cell *const calc_pages[] = {calc_cells_alt, calc_cells_alt2};

const struct view_def view_calc = {
    .cells = calc_cells,
    .pages = calc_pages,
    .num_pages = 3,
    .build = calc_update_display,
    .on_hold = hold_calc,
    .on_enter = calc_on_enter,
};

static void tap_calc_to_bin(int cell) {
  ARG_UNUSED(cell);
  if (!bin_mode) convert_to_binary();
  bin_mode = true;
  cur_page = 2; // page 3
  build_view(&view_calc);
}

static void tap_calc_to_dec(int cell) {
  ARG_UNUSED(cell);
  if (bin_mode) convert_to_decimal();
  bin_mode = false;
  cur_page = 0; // page 1
  build_view(&view_calc);
}

static void tap_calc_to_page2(int cell) {
  ARG_UNUSED(cell);
  if (bin_mode) convert_to_decimal();
  bin_mode = false;
  cur_page = 1; // page 2
  build_view(&view_calc);
}
