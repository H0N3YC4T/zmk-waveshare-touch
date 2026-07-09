#include "calc_internal.h"
#include <string.h>

void tap_calc_0(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('7'); calc_update_display(); }
void tap_calc_1(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('8'); calc_update_display(); }
void tap_calc_2(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('9'); calc_update_display(); }
void tap_calc_3(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('+'); calc_update_display(); }
void tap_calc_4(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('4'); calc_update_display(); }
void tap_calc_5(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('5'); calc_update_display(); }
void tap_calc_6(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('6'); calc_update_display(); }
void tap_calc_7(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('-'); calc_update_display(); }
void tap_calc_8(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('1'); calc_update_display(); }
void tap_calc_9(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('2'); calc_update_display(); }
void tap_calc_10(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('3'); calc_update_display(); }
void tap_calc_11(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('*'); calc_update_display(); }
void tap_calc_13(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('0'); calc_update_display(); }
void tap_calc_14(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_tap_eval(); calc_update_display(); }
void tap_calc_15(int cell) { ARG_UNUSED(cell); calc_ensure_binary(); calc_push('/'); calc_update_display(); }

void tap_calc_backspace_cb(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_tap_backspace();
  calc_update_display();
}
void tap_calc_dot(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('.');
  calc_update_display();
}
void tap_calc_bracket(int cell) {
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

void tap_calc_percent(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('%');
  calc_update_display();
}
void tap_calc_factorial(int cell) {
  ARG_UNUSED(cell);
  calc_ensure_binary();
  calc_push('!');
  calc_update_display();
}

void tap_calc_to_bin(int cell) {
  ARG_UNUSED(cell);
  bin_mode = true;
  bin_converted = false;
  cur_page = 2; // page 3
  build_view(&view_calc);
}

void tap_calc_to_dec(int cell) {
  ARG_UNUSED(cell);
  bin_mode = false;
  bin_converted = false;
  cur_page = 0; // page 1
  build_view(&view_calc);
}

void tap_calc_to_page2(int cell) {
  ARG_UNUSED(cell);
  bin_mode = false;
  bin_converted = false;
  cur_page = 1; // page 2
  build_view(&view_calc);
}

const struct page_cell calc_cells[] = {
    {0, 0, 1, 3, "0", NULL, COLOR_ACCENT, ACT_GO_VIEW,
     .arg.view = &view_home}, // Display row
    {0, 3, 1, 1, LV_SYMBOL_BACKSPACE, NULL, COLOR_RED, ACT_CUSTOM,
     .arg.func = tap_calc_backspace_cb}, // Back/clear

    {1, 0, 1, 1, "7", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_0},
    {1, 1, 1, 1, "8", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_1},
    {1, 2, 1, 1, "9", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_2},
    {1, 3, 1, 1, "+", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_3},

    {2, 0, 1, 1, "4", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_4},
    {2, 1, 1, 1, "5", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_5},
    {2, 2, 1, 1, "6", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_6},
    {2, 3, 1, 1, "-", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_7},

    {3, 0, 1, 1, "1", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_8},
    {3, 1, 1, 1, "2", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_9},
    {3, 2, 1, 1, "3", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_10},
    {3, 3, 1, 1, "*", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_11},

    {4, 0, 1, 1, LV_SYMBOL_NEXT, NULL, COLOR_YELLOW, ACT_CUSTOM,
     .arg.func = tap_calc_to_page2}, // page 2
    {4, 1, 1, 1, "0", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_13},
    {4, 2, 1, 1, "=", NULL, COLOR_GREEN, ACT_CUSTOM, .arg.func = tap_calc_14},
    {4, 3, 1, 1, "/", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_15},

    {0}};

const struct page_cell calc_cells_alt[] = {
    {0, 0, 1, 3, "0", NULL, COLOR_ACCENT, ACT_GO_VIEW,
     .arg.view = &view_home}, // Display row
    {0, 3, 1, 1, LV_SYMBOL_BACKSPACE, NULL, COLOR_RED, ACT_CUSTOM,
     .arg.func = tap_calc_backspace_cb}, // Back/clear

    {1, 0, 1, 1, "7", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_0},
    {1, 1, 1, 1, "8", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_1},
    {1, 2, 1, 1, "9", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_2},
    {1, 3, 1, 1, "()", NULL, COLOR_ACCENT, ACT_CUSTOM,
     .arg.func = tap_calc_bracket},

    {2, 0, 1, 1, "4", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_4},
    {2, 1, 1, 1, "5", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_5},
    {2, 2, 1, 1, "6", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_6},
    {2, 3, 1, 1, "%", NULL, COLOR_ACCENT, ACT_CUSTOM,
     .arg.func = tap_calc_percent},

    {3, 0, 1, 1, "1", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_8},
    {3, 1, 1, 1, "2", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_9},
    {3, 2, 1, 1, "3", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_10},
    {3, 3, 1, 1, "!", NULL, COLOR_ACCENT, ACT_CUSTOM,
     .arg.func = tap_calc_factorial},

    {4, 0, 1, 1, LV_SYMBOL_PREV, NULL, COLOR_YELLOW, ACT_CUSTOM,
     .arg.func = tap_calc_to_dec}, // return page 1
    {4, 1, 1, 1, "0", NULL, COLOR_PRIMARY, ACT_CUSTOM, .arg.func = tap_calc_13},
    {4, 2, 1, 1, LV_SYMBOL_NEXT, NULL, COLOR_YELLOW, ACT_CUSTOM, .arg.func = tap_calc_to_bin},
    {4, 3, 1, 1, ".", NULL, COLOR_ACCENT, ACT_CUSTOM, .arg.func = tap_calc_dot},

    {0}};
