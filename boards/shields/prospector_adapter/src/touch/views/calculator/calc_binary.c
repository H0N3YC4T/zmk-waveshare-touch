#include "calc_internal.h"
#include <string.h>

void tap_calc_ws(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  if (bin_word_size == 64)
    bin_word_size = 32;
  else if (bin_word_size == 32)
    bin_word_size = 16;
  else if (bin_word_size == 16)
    bin_word_size = 8;
  else
    bin_word_size = 64;
  bin_converted = false;

  // Re-evaluate to remask correctly
  if (calc_expr[0] != '\0') {
    eval_okay = true;
    calc_cp = &calc_expr[0];
    float r = parse_expr();
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

void tap_calc_and(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('&');
  calc_update_display();
}
void tap_calc_or(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('|');
  calc_update_display();
}
void tap_calc_xor(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('^');
  calc_update_display();
}
void tap_calc_not(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('~');
  calc_update_display();
}
void tap_calc_lsh(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('<');
  calc_update_display();
}
void tap_calc_rsh(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('>');
  calc_update_display();
}

const struct page_cell calc_cells_alt2[] = {
    {0, 0, 1, 3, "0", NULL, COLOR_ACCENT, ACT_GO_VIEW,
     .arg.view = &view_home}, // Display row
    {0, 3, 1, 1, LV_SYMBOL_BACKSPACE, NULL, COLOR_RED, ACT_CUSTOM,
     .arg.func = tap_calc_backspace_cb}, // Back/clear

    {1, 0, 1, 1, "&", NULL, COLOR_PRIMARY, ACT_CUSTOM,
     .arg.func = tap_calc_and},
    {1, 1, 1, 1, "|", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_or},
    {1, 2, 1, 1, "^", NULL, COLOR_PRIMARY, ACT_CUSTOM,
     .arg.func = tap_calc_xor},
    {1, 3, 1, 1, "+", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_3},

    {2, 0, 1, 1, "~", NULL, COLOR_PRIMARY, ACT_CUSTOM,
     .arg.func = tap_calc_not},
    {2, 1, 1, 1, "<", NULL, COLOR_PRIMARY, ACT_CUSTOM,
     .arg.func = tap_calc_lsh},
    {2, 2, 1, 1, ">", NULL, COLOR_PRIMARY, ACT_CUSTOM,
     .arg.func = tap_calc_rsh},
    {2, 3, 1, 1, "-", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_7},

    {3, 0, 1, 1, "1", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_8},
    {3, 1, 1, 1, "%", NULL, COLOR_PRIMARY, ACT_CUSTOM,
     .arg.func = tap_calc_percent},
    {3, 2, 1, 1, "WS", NULL, COLOR_YELLOW, ACT_CUSTOM, .arg.func = tap_calc_ws},
    {3, 3, 1, 1, "*", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_11},

    {4, 0, 1, 1, LV_SYMBOL_REFRESH, NULL, COLOR_YELLOW, ACT_CUSTOM,
     .arg.func = tap_calc_to_dec},
    {4, 1, 1, 1, "0", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_13},
    {4, 2, 1, 1, "=", NULL, COLOR_GREEN, ACT_CUSTOM, .arg.func = tap_calc_14},
    {4, 3, 1, 1, "/", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_15},

    {0}};
