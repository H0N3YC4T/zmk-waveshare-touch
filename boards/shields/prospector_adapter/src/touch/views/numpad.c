/* -------------------------------- NUMPAD ----------------------------------- */
/* 4x4 calc grid; operators (blue) in column 3; cell 12 = Back, 14 = Enter. Keys
 * are true HID Keypad codes (KP_*), not the main-row digits/symbols -- so this
 * behaves as an actual numpad for apps/fields that distinguish the two (numeric
 * entry, spreadsheets, RDP), not just a second way to type "7890". Back -> HOME. */

#include "../touch_ui.h"

void build_numpad(void)
{
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
      LV_SYMBOL_UP,
      "0",
      LV_SYMBOL_NEW_LINE,
      "/",
  };
  for (int c = 0; c < 16; c++)
  {
    uint32_t color = (c == 12)                 ? COLOR_RED
                     : (c % 4 == 3 || c == 14) ? COLOR_ACCENT
                                               : COLOR_PRIMARY;
    draw_cell(c / 4, c % 4, 1, lbls[c], color);
  }
}

void tap_numpad(int cell)
{
  /* Index 12 (Back) is 0 and handled first, so the np[cell] guard only skips
   * it -- every real key code is non-zero. */
  static const uint32_t np[16] = {KP_N7, KP_N8, KP_N9, KP_PLUS,
                                  KP_N4, KP_N5, KP_N6, KP_MINUS,
                                  KP_N1, KP_N2, KP_N3, KP_MULTIPLY,
                                  0, KP_N0, KP_ENTER, KP_DIVIDE};
  if (cell == 12)
  {
    show_view(VIEW_HOME);
  }
  else if (cell >= 0 && cell <= 15 && np[cell])
  {
    send_key(np[cell]);
  }
}
