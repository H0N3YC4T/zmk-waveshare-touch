/* --------------------------------- CALC ------------------------------------ */
/* On-dongle calculator (hold 123 on HOME). */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../touch_ui.h"

static char calc_expr[40];
static const char *calc_cp;
static bool calc_shown;
static bool eval_okay;

static const char btn[16] = {
    '7',
    '8',
    '9',
    '+',
    '4',
    '5',
    '6',
    '-',
    '1',
    '2',
    '3',
    '*',
    '\b',
    '0',
    '=',
    '/',
};

static const char *const lbls[16] = {
    "7",
    "8",
    "9",
    "+",
    "4",
    "5",
    "6",
    "-",
    "1",
    "2",
    "3",
    "*",
    LV_SYMBOL_BACKSPACE,
    "0",
    "=",
    "/",
};

/* DISPLAY FUNCTIONS */
void build_calc(void)
{
  draw_cell(0, 0, 4, calc_expr[0] ? calc_expr : "0", COLOR_ACCENT); /* display, spans row */

  for (int c = 0; c < 16; c++)
  {
    uint32_t color = (c == 12)      ? COLOR_RED     /* backspace */
                     : (c == 14)    ? COLOR_GREEN   /* = evaluate */
                     : (c % 4 == 3) ? COLOR_ACCENT    /* operators */
                                    : COLOR_PRIMARY; /* digits */
    draw_cell(1 + c / 4, c % 4, 1, lbls[c], color);
  }
}

void tap_calc(int cell)
{
  // Display row pressed, exit to HOME
  if (cell >= 0 && cell <= 3)
  {
    show_view(VIEW_HOME);
    return;
  }
  else
    calc_controller(cell);

  build_view(VIEW_CALC);
}

/* CALCULATOR FUNCTIONS */
static double parse_expr(void)
{
  double a = parse_term();
  while (eval_okay && (*calc_cp == '+' || *calc_cp == '-'))
  {
    char op = *calc_cp++;
    double b = parse_term();
    a = (op == '+')
            ? a + b
            : a - b;
  }
  return a;
}

static double parse_term(void)
{
  double a = parse_factor();
  while (eval_okay && (*calc_cp == '*' || *calc_cp == '/'))
  {
    char op = *calc_cp++;
    double b = parse_factor();

    if (op == '*')
    {
      a *= b;
    }
    else if (b == 0)
    {
      eval_okay = false;
      a = 0;
    }
    else
    {
      a /= b;
    }
  }
  return a;
}

static double parse_factor(void)
{
  double sum = 0;
  bool num_empty = true;

  // for each digit
  while (*calc_cp >= '0' && *calc_cp <= '9')
  {
    sum = sum * 10 + (*calc_cp - '0');
    calc_cp++;
    num_empty = false;
  }
  // If no digits were found
  if (num_empty)
    eval_okay = false;
  return sum;
}

/* USER INPUT HANDLER */
static void calc_push(char ch)
{
  if (calc_shown) // If a result is displayed
  {
    // clear on number input
    if (ch >= '0' && ch <= '9')
    {
      calc_expr[0] = '\0';
    }
    calc_shown = false;
  }

  size_t n = strlen(calc_expr);
  if (n < sizeof(calc_expr) - 1)
  {
    calc_expr[n] = ch;
    calc_expr[n + 1] = '\0';
  }
}

static void calc_tap_backspace(void)
{
  if (calc_shown)
  {
    calc_expr[0] = '\0';
    calc_shown = false;
  }
  else
  {
    size_t n = strlen(calc_expr);
    if (n)
    {
      calc_expr[n - 1] = '\0';
    }
  }
}

static void calc_print_result(double result)
{
  int is_negative = result < 0;
  result = is_negative ? -result : result;

  int whole = (int)result;
  int fraction = (int)((result - whole) * 100 + 0.5);
  if (fraction >= 100) {
      whole++;
      fraction = 0;
  }

  snprintf(calc_expr, sizeof(calc_expr), "%s%d.%02d", 
           (is_negative && (whole || fraction)) ? "-" : "", whole, fraction);

  int len = strlen(calc_expr);
  while (len > 0 && calc_expr[len - 1] == '0') calc_expr[--len] = '\0';
  if (len > 0 && calc_expr[len - 1] == '.') calc_expr[--len] = '\0';
}

static void calc_tap_eval(void)
{
  double result = 0;
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
}

static void calc_controller(int cell)
{
  int b = cell - 4; // Exclude display row

  // Invalid button index, do nothing
  if (b < 0 || b > 15)
    return;

  char ch = btn[b];

  // Backspace is pressed
  if (ch == '\b')
    calc_tap_backspace();

  // Eval is pressed
  else if (ch == '=')
    calc_tap_eval();

  // Any other button is pressed
  else
    calc_push(ch);
}
