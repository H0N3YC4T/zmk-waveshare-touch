#include "calc_internal.h"
#include <stdio.h>

char calc_expr[40];
const char *calc_cp;
bool calc_shown;
bool eval_okay;
bool bracket_open;

bool bin_mode;
bool bin_converted;
int bin_word_size = 8;




static const struct page_cell *const calc_pages[] = {calc_cells_alt,
                                                     calc_cells_alt2};

/* FORWARD DECLARATIONS */
static float parse_bitwise_or(void);
static float parse_bitwise_xor(void);
static float parse_bitwise_and(void);
static float parse_shift(void);
static float parse_add_sub(void);
static float parse_term(void);
static float parse_factor(void);

/* DISPLAY FUNCTIONS */
void calc_update_display(void) {
  if (cur_view_btns[0]) {
    lv_obj_t *l = lv_obj_get_child(cur_view_btns[0], 0);
    if (l) {
      if (bin_mode) {
        lv_label_set_text(l, calc_expr[0] ? calc_expr : "0");
      } else {
        lv_label_set_text(l, calc_expr[0] ? calc_expr : "0");
      }
    }
  }
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

/* CALCULATOR FUNCTIONS */
float parse_expr(void) { return parse_bitwise_or(); }

static float parse_bitwise_or(void) {
  float a = parse_bitwise_xor();
  while (eval_okay && (*calc_cp == '|')) {
    calc_cp++;
    float b = parse_bitwise_xor();
    a = (float)((long long)a | (long long)b);
  }
  return a;
}

static float parse_bitwise_xor(void) {
  float a = parse_bitwise_and();
  while (eval_okay && (*calc_cp == '^')) {
    calc_cp++;
    float b = parse_bitwise_and();
    a = (float)((long long)a ^ (long long)b);
  }
  return a;
}

static float parse_bitwise_and(void) {
  float a = parse_shift();
  while (eval_okay && (*calc_cp == '&')) {
    calc_cp++;
    float b = parse_shift();
    a = (float)((long long)a & (long long)b);
  }
  return a;
}

static float parse_shift(void) {
  float a = parse_add_sub();
  while (eval_okay && (*calc_cp == '<' || *calc_cp == '>')) {
    char op = *calc_cp++;
    float b = parse_add_sub();
    long long shift_amt = (long long)b;
    if (shift_amt < 0 || shift_amt > 63) {
      eval_okay = false;
    } else {
      if (op == '<')
        a = (float)((long long)a << shift_amt);
      else
        a = (float)((long long)a >> shift_amt);
    }
  }
  return a;
}

static float parse_add_sub(void) {
  float a = parse_term();
  while (eval_okay && (*calc_cp == '+' || *calc_cp == '-')) {
    char op = *calc_cp++;
    float b = parse_term();
    a = (op == '+') ? a + b : a - b;
  }
  return a;
}

static float parse_term(void) {
  float a = parse_factor();
  while (eval_okay && (*calc_cp == '*' || *calc_cp == '/' || *calc_cp == '%')) {
    char op = *calc_cp++;
    float b = parse_factor();

    if (op == '*') {
      a *= b;
    } else if (op == '%') {
      if (b == 0) {
        eval_okay = false;
        a = 0;
      } else
        a = (float)((long long)a % (long long)b);
    } else if (b == 0) {
      eval_okay = false;
      a = 0;
    } else {
      a /= b;
    }
  }
  return a;
}

static float parse_factor(void) {
  float sum = 0;
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
    if (*calc_cp == ')') {
      calc_cp++;
    } else {
      eval_okay = false;
    }
    sum = is_negative ? -sum : sum;
    if (is_bitwise_not)
      sum = (float)(~(long long)sum);
    return sum;
  }

  if (bin_mode) {
    while (*calc_cp == '0' || *calc_cp == '1') {
      sum = sum * 2 + (*calc_cp - '0');
      calc_cp++;
      num_empty = false;
    }
  } else {
    // for each digit
    while (*calc_cp >= '0' && *calc_cp <= '9') {
      sum = sum * 10 + (*calc_cp - '0');
      calc_cp++;
      num_empty = false;
    }

    // for decimal point
    if (*calc_cp == '.') {
      calc_cp++;
      float fraction = 0.1f;
      while (*calc_cp >= '0' && *calc_cp <= '9') {
        sum += (*calc_cp - '0') * fraction;
        fraction /= 10.0f;
        calc_cp++;
        num_empty = false;
      }
    }
  }

  if (*calc_cp == '!') {
    calc_cp++;
    if (sum >= 0 && sum == (long long)sum) {
      long long n = (long long)sum;
      if (n > 20) {
        eval_okay = false;
      } else {
        float f = 1;
        for (long long i = 2; i <= n; i++)
          f *= i;
        sum = f;
      }
    } else {
      eval_okay = false;
    }
  }

  // If no digits were found
  if (num_empty)
    eval_okay = false;
  sum = is_negative ? -sum : sum;
  if (is_bitwise_not)
    sum = (float)(~(long long)sum);
  return sum;
}

/* USER INPUT HANDLER */
void calc_push(char ch) {
  if (calc_shown) // If a result is displayed
  {
    // clear on number or decimal input
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

void calc_tap_backspace(void) {
  if (calc_shown) {
    calc_expr[0] = '\0';
    calc_shown = false;
    bracket_open = false;
  } else {
    size_t n = strlen(calc_expr);
    if (n) {
      char deleted = calc_expr[n - 1];
      calc_expr[n - 1] = '\0';
      if (deleted == '(')
        bracket_open = false;
      else if (deleted == ')')
        bracket_open = true;
    }
  }
}

static void calc_print_result(float result) {
  if (bin_mode) {
    long long val = (long long)(result > 0 ? result + 0.5f : result - 0.5f);
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
  int fraction = (int)((result - whole) * 100.0f + 0.5f);
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

void calc_tap_eval(void) {
  if (bracket_open) {
    calc_push(')');
  }

  float result = 0;
  eval_okay = true;
  calc_cp = &calc_expr[0];

  // Eval if not empty
  if (*calc_cp != '\0')
    result = parse_expr();

  // Print result if valid
  if (eval_okay) {
    calc_print_result(result);
  }
  // Print error if invalid
  else
    snprintf(calc_expr, sizeof(calc_expr), "Error");

  calc_shown = true;
  bracket_open = false;
}

void calc_ensure_binary(void) {
  if (bin_mode && !bin_converted) {
    if (calc_expr[0] != '\0') {
      eval_okay = true;
      calc_cp = &calc_expr[0];
      float r = parse_expr();
      if (eval_okay) {
        long long val = (long long)(r > 0 ? r + 0.5f : r - 0.5f);
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
          // reverse string
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
    bin_converted = true;
    calc_shown = true;
    bracket_open = false;
  }
}



static void calc_on_enter(void) {
  calc_expr[0] = '\0';
  calc_shown = false;
  bracket_open = false;
  bin_mode = false;
  bin_converted = false;
}

const struct view_def view_calc = {
    .cells = calc_cells,
    .pages = calc_pages,
    .num_pages = 3,
    .build = calc_update_display,
    .on_hold = hold_calc,
    .on_enter = calc_on_enter,
};
