/* -------------------------------- HOME ------------------------------------ */

#include <zephyr/device.h>
#include "../touch_ui.h"

static const struct view_def view_home_alt;

static void tap_home_pad(int cell)
{
  if (touch_pad_count() > 0)
  {
    show_view(&view_pad);
  }
}

static void tap_studio_unlock(int cell)
{
  ARG_UNUSED(cell);
#if DT_NODE_EXISTS(DT_NODELABEL(studio_unlock))
  fire_macro(DEVICE_DT_NAME(DT_NODELABEL(studio_unlock)));
#endif
}

static void tap_bootloader(int cell)
{
  ARG_UNUSED(cell);
#if DT_NODE_EXISTS(DT_NODELABEL(bootloader))
  fire_macro(DEVICE_DT_NAME(DT_NODELABEL(bootloader)));
#endif
}

static void tap_clear_mods(int cell)
{
  ARG_UNUSED(cell);
  pending_mods = 0;
  build_view(cur_view);
}

static void hold_home(int cell)
{
  switch (cell)
  {
  case 0:
    tap_studio_unlock(cell);
    break;
  case 1:
    tap_bootloader(cell);
    break;
  case 2:
    show_view(&view_calc);
    break;
  case 4:
    show_view(&view_home_alt);
    break;
  case 5:
    show_view(&view_scrollpad);
    break;
  case 6:
    tap_clear_mods(cell);
    break;
  case 7:
    show_view(&view_clipboard);
    break;
  case 8:
    show_view(&view_media2);
    break;
  default:
    tap_declarative(cell);
    break;
  }
}

static const struct page_cell home_cells[] = {
    {0, 0, 1, 1, NULL, &icon_fkeys, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_fkeys},
    {0, 1, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_normal},
    {0, 2, 1, 1, NULL, &icon_numpad, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_numpad},

    {1, 0, 1, 1, NULL, &icon_symbols, THEME_SECONDARY, ACT_GO_VIEW, .arg.view = &view_symbols},
    {1, 1, 1, 1, NULL, &icon_settings, THEME_FOCUS, ACT_GO_VIEW, .arg.view = &view_settings},
    {1, 2, 1, 1, NULL, &icon_trackpad, THEME_SECONDARY, ACT_GO_VIEW, .arg.view = &view_trackpad},

    {2, 0, 1, 1, "MOD", &icon_modkeys, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_modifiers},
    {2, 1, 1, 1, NULL, &icon_keyboard, THEME_SECONDARY, ACT_CUSTOM, .arg.func = tap_home_pad},
    {2, 2, 1, 1, NULL, &icon_audio, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_media},
    {0}};

const struct view_def view_home = {
    .cells = home_cells,
    .keeps_mods = true,
    .on_hold = hold_home,
    .idle_timeout = true,
};

/* ------------------------------ HOLD MODE ---------------------------------- */
/* alternate home reached by holding SETTINGS: icons show each button's hold
 * action, tap and hold swap, and idling reverts to the normal home */

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
    {0, 0, 1, 1, NULL, &icon_unlock, THEME_PRIMARY, ACT_CUSTOM, .arg.func = tap_studio_unlock},
    {0, 1, 1, 1, NULL, &icon_boot, THEME_DENY, ACT_CUSTOM, .arg.func = tap_bootloader},
    {0, 2, 1, 1, NULL, &icon_calc, THEME_PRIMARY, ACT_GO_VIEW, .arg.view = &view_calc},

    /* symbols has no hold action -- muted, tap does nothing */
    {1, 0, 1, 1, NULL, &icon_symbols, THEME_MUTED, ACT_NONE},
    {1, 1, 1, 1, NULL, &icon_swap, THEME_FOCUS, ACT_CUSTOM, .arg.func = tap_exit_alt},
    {1, 2, 1, 1, NULL, &icon_scroll, THEME_SECONDARY, ACT_GO_VIEW, .arg.view = &view_scrollpad},

    {2, 0, 1, 1, NULL, &icon_modclear, THEME_DENY, ACT_CUSTOM, .arg.func = tap_clear_mods},
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
