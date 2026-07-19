
/* ------------------------------ HOLD MODE ---------------------------------- */
/* alternate home reached by holding SETTINGS: icons show each button's hold action */
#include <zephyr/device.h>
#include "../touch_ui.h"

static void tap_exit_alt(int cell)
{
  ARG_UNUSED(cell);
  show_view(&view_home);
}

/* the normal home taps, reached by holding in hold mode */
static void hold_home_alt(int cell)
{
  switch (cell)
  {
  case 0:
    show_view(&view_fkeys);
    break;
  case 1:
    show_view(&view_normal);
    break;
  case 2:
    show_view(&view_numpad);
    break;
  case 3:
    show_view(&view_symbols);
    break;
  case 4:
    show_view(&view_settings);
    break;
  case 5:
    show_view(&view_trackpad);
    break;
  case 6:
    show_view(&view_modifiers);
    break;
  case 7:
    tap_home_pad(cell);
    break;
  case 8:
    show_view(&view_media);
    break;
  default:
    break;
  }
}

static const struct page_cell home_alt_cells[] = {
    {0, 0, 1, 1, NULL, &icon_unlock, THEME_INCREMENT, ACT_CUSTOM, .arg.func = tap_studio_unlock},
    {0, 1, 1, 1, NULL, &icon_boot, THEME_INCREMENT, ACT_CUSTOM, .arg.func = tap_bootloader},
    {0, 2, 1, 1, NULL, &icon_calc, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_calc},

    {1, 0, 1, 1, NULL, &icon_symbols, THEME_MUTED, ACT_NONE}, /* symbols has no hold action */
    {1, 1, 1, 1, NULL, &icon_swap, THEME_FOCUS, ACT_CUSTOM, .arg.func = tap_exit_alt},
    {1, 2, 1, 1, NULL, &icon_scroll, THEME_SECONDARY, ACT_GO_VIEW, .arg.view = &view_scrollpad},

    {2, 0, 1, 1, NULL, &icon_modclear, THEME_PRIMARY, ACT_CUSTOM, .arg.func = tap_clear_mods},
    {2, 1, 1, 1, NULL, &icon_clipboard, THEME_SECONDARY, ACT_GO_VIEW, .arg.view = &view_clipboard},
    {2, 2, 1, 1, NULL, &icon_mixer, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_media2},
    {0}};

static const struct view_def view_home_alt = {
    .cells = home_alt_cells,
    .keeps_mods = true,
    .on_hold = hold_home_alt,
    .idle_timeout = true,
    .timeout_ms = HOME_ALT_TIMEOUT_MS,
    .timeout_view = &view_home,
};
