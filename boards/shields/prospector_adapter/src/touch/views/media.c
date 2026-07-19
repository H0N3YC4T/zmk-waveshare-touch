/* -------------------------------- MEDIA ------------------------------------ */

#include "../touch_ui.h"

/* hold play/pause = the extras page (mute/stop/seek/player launch) */
static void hold_media(int cell)
{
  bool on_play = (ui_rot & 1) ? (cell == 14 || cell == 15) : (cell == 4);
  if (on_play)
  {
    show_view(&view_media2);
  }
  else
  {
    tap_declarative(cell);
  }
}

static const struct page_cell media_cells[] = {
    {0, 0, 1, 1, NULL, &icon_voldown, THEME_DECREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_0"},
    {0, 1, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},
    {0, 2, 1, 1, NULL, &icon_volup, THEME_INCREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_2"},

    {1, 0, 1, 1, NULL, &icon_prev, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_4"},
    {1, 1, 1, 1, NULL, &icon_play, THEME_SECONDARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_3"},
    {1, 2, 1, 1, NULL, &icon_next, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_5"},
    {0}};

static const struct page_cell media_cells_portrait[] = {
    {0, 0, 1, 6, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_home},

    {1, 0, 1, 3, NULL, &icon_voldown, THEME_DECREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_0"},
    {1, 3, 1, 3, NULL, &icon_volup, THEME_INCREMENT, ACT_FIRE_MACRO, .arg.macro = "touch_macro_2"},

    {2, 0, 1, 2, NULL, &icon_prev, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_4"},
    {2, 2, 1, 2, NULL, &icon_play, THEME_SECONDARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_3"},
    {2, 4, 1, 2, NULL, &icon_next, THEME_PRIMARY, ACT_FIRE_MACRO, .arg.macro = "touch_macro_5"},
    {0}};

const struct view_def view_media = {
    .cells = media_cells,
    .cells_portrait = media_cells_portrait,
    .on_hold = hold_media,
};

/* ----------------------------- MEDIA EXTRAS -------------------------------- */

static const struct page_cell media2_cells[] = {
    {0, 0, 1, 1, NULL, &icon_mute, THEME_FOCUS, ACT_FIRE_MACRO, .arg.macro = "touch_macro_1"},
    {0, 1, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_media},
    {0, 2, 1, 1, NULL, &icon_stop, THEME_DENY, ACT_SEND_KEY, .arg.keycode = C_STOP},

    {1, 0, 1, 1, NULL, &icon_rew, THEME_DECREMENT, ACT_SEND_KEY, .arg.keycode = C_RW},
    {1, 1, 1, 1, NULL, &icon_music, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = C_AL_CCC},
    {1, 2, 1, 1, NULL, &icon_ff, THEME_INCREMENT, ACT_SEND_KEY, .arg.keycode = C_FF},
    {0}};

static const struct page_cell media2_cells_portrait[] = {
    {0, 0, 1, 6, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_media},

    {1, 0, 1, 3, NULL, &icon_mute, THEME_FOCUS, ACT_FIRE_MACRO, .arg.macro = "touch_macro_1"},
    {1, 3, 1, 3, NULL, &icon_stop, THEME_DENY, ACT_SEND_KEY, .arg.keycode = C_STOP},

    {2, 0, 1, 2, NULL, &icon_rew, THEME_DECREMENT, ACT_SEND_KEY, .arg.keycode = C_RW},
    {2, 2, 1, 2, NULL, &icon_music, THEME_PRIMARY, ACT_SEND_KEY, .arg.keycode = C_AL_CCC},
    {2, 4, 1, 2, NULL, &icon_ff, THEME_INCREMENT, ACT_SEND_KEY, .arg.keycode = C_FF},
    {0}};

const struct view_def view_media2 = {
    .cells = media2_cells,
    .cells_portrait = media2_cells_portrait,
};
