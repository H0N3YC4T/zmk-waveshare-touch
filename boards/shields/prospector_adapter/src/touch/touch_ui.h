#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>

#include "theme.h"

/* ------------------------------- constants -------------------------------- */

#define SCR_W 280
#define SCR_H 240
#define TOUCH_TIMEOUT_MS 30000
#define TOUCH_HOLD_MS 500 /* press-and-lift >= this = a hold, not a tap */
#define BRIGHTNESS_STEP 10
#define KEYS_PER_PAGE 7

#define SETTINGS_SENS_MAX 10
#define SETTINGS_BRIGHT_MAX 100
#define SETTINGS_BRIGHT_MIN 5

#define TP_SCROLL_ZONE 240

#define GLASS_RADIUS 44
#define UI_PAD 5
#define BTN_RADIUS 14

enum action_type
{
  ACT_NONE,
  ACT_GO_VIEW,
  ACT_SEND_KEY,
  ACT_CUSTOM,
  ACT_FIRE_MACRO,
  ACT_NEXT_PAGE,
  ACT_PREV_PAGE,
  ACT_CUSTOM_VAL
};

struct view_def; // Forward declaration for page_cell arg

struct page_cell
{
  int row;
  int col;
  int row_span;
  int col_span;
  const char *label;
  const lv_image_dsc_t *icon;
  enum theme_role color;
  enum action_type action;
  union
  {
    const struct view_def *view;
    uint32_t keycode;
    const char *macro;
    void (*func)(int cell);
    struct
    {
      void (*cb)(int val);
      int val;
    } custom;
  } arg;
};

struct view_def
{
  const struct page_cell *cells;                 /* declarative layout and actions; NULL-terminated */
  const struct page_cell *cells_portrait;        /* optional explicit portrait layout override */
  const struct page_cell *const *pages;          /* array of paginated landscape layouts */
  const struct page_cell *const *pages_portrait; /* array of paginated portrait overrides */
  uint8_t num_pages;                             /* TOTAL pages incl. the base cells page; pages[] holds N-1 */
  void (*build)(void);                           /* renderer; NULL = nothing to draw (NORMAL) */
  bool idle_timeout;                             /* return to NORMAL after TOUCH_TIMEOUT_MS idle */
  bool keeps_mods;                               /* armed one-shot mods survive entering this view */
  void (*on_hold)(int cell);                     /* long-press handler; NULL = holds act as taps */
  void (*on_enter)(void);                        /* called once when the view is shown */
};

/* ----------------------------- shared state ------------------------------- */
extern lv_obj_t *touch_overlay;         /* full-screen touch UI layer (touch_nav.c) */
extern lv_obj_t *cur_view_btns[32];     /* cached declarative button objects for mutation */
extern const struct view_def *cur_view; /* current view (touch_nav.c) */
extern int cur_page;                    /* page of the paginated key screens (touch_nav.c) */
extern uint8_t pending_mods;            /* one-shot mods, applied to the next key (touch_keys.c) */
extern int grid_rows;                   /* current screen's grid (touch_draw.c) */
extern int grid_cols;
extern uint8_t ui_rot; /* 0..3 = 0/90/180/270 deg CW (touch_rotation.c) */

static inline lv_coord_t scr_w(void) { return (ui_rot & 1) ? SCR_H : SCR_W; }
static inline lv_coord_t scr_h(void) { return (ui_rot & 1) ? SCR_W : SCR_H; }

// Home Icons
extern const lv_image_dsc_t icon_fkeys __weak;
extern const lv_image_dsc_t icon_numpad __weak;
extern const lv_image_dsc_t icon_symbols __weak;
extern const lv_image_dsc_t icon_settings __weak;
extern const lv_image_dsc_t icon_trackpad __weak;
extern const lv_image_dsc_t icon_modkeys __weak;
extern const lv_image_dsc_t icon_keyboard __weak;
extern const lv_image_dsc_t icon_audio __weak;

// Shortcut Icons
extern const lv_image_dsc_t icon_terminal __weak;
extern const lv_image_dsc_t icon_list __weak;
extern const lv_image_dsc_t icon_browser __weak;
extern const lv_image_dsc_t icon_desktop __weak;
extern const lv_image_dsc_t icon_notes __weak;

// Clipboard Icons
extern const lv_image_dsc_t icon_paste __weak;
extern const lv_image_dsc_t icon_copy __weak;
extern const lv_image_dsc_t icon_cut __weak;

// Mod Keys
extern const lv_image_dsc_t icon_alt __weak;
extern const lv_image_dsc_t icon_shift __weak;
extern const lv_image_dsc_t icon_gui __weak;

// Settings Icons
extern const lv_image_dsc_t icon_sens __weak;
extern const lv_image_dsc_t icon_bright __weak;
extern const lv_image_dsc_t icon_rotate __weak;
extern const lv_image_dsc_t icon_theme __weak;

// Media Icons
extern const lv_image_dsc_t icon_play __weak;
extern const lv_image_dsc_t icon_next __weak;
extern const lv_image_dsc_t icon_prev __weak;
extern const lv_image_dsc_t icon_voldown __weak;
extern const lv_image_dsc_t icon_volup __weak;

// General Icons
extern const lv_image_dsc_t icon_plus __weak;
extern const lv_image_dsc_t icon_minus __weak;
extern const lv_image_dsc_t icon_close __weak;
extern const lv_image_dsc_t icon_newline __weak;
extern const lv_image_dsc_t icon_newline_32 __weak;
extern const lv_image_dsc_t icon_backspace __weak;

// Arrow Icons
extern const lv_image_dsc_t icon_up __weak;
extern const lv_image_dsc_t icon_up_32 __weak;
extern const lv_image_dsc_t icon_down __weak;
extern const lv_image_dsc_t icon_left __weak;
extern const lv_image_dsc_t icon_left_32 __weak;
extern const lv_image_dsc_t icon_right __weak;

/* ------------------------------- functions -------------------------------- */
/* touch_draw.c */
lv_obj_t *draw_cell(int row, int col, int w_cells, const char *text, enum theme_role accent);
lv_obj_t *draw_cell_ext(int row, int col, int w_cells, int h_cells, const char *text, enum theme_role accent, bool filled);
lv_obj_t *draw_cell_icon(int row, int col, const lv_image_dsc_t *icon, const char *fallback, enum theme_role accent);
lv_obj_t *draw_cell_icon_ext(int row, int col, int w_cells, int h_cells, const lv_image_dsc_t *icon, const char *fallback, enum theme_role accent);
void cell_child_set_color(lv_obj_t *btn, enum theme_role role); /* label text or image recolor */

/* touch_main.c */
void build_view(const struct view_def *v);
void tap_declarative(int cell);
bool ui_has_action(int cell);

/* touch_nav.c */
void show_view(const struct view_def *v);
void touch_ui_attach(lv_obj_t *screen); /* create the overlay + the drain timer */

/* touch_keys.c */
void send_key(uint32_t keycode);
void fire_macro(const char *dev);
int touch_pad_count(void); /* bound PAD buttons (zmk,prospector-touch-pad) */
void fire_pad(int idx);    /* invoke PAD binding idx (M1 = 0) */

/* touch_rotation.c */
void settings_apply_rotation(void);

/* touch_prefs.c: brightness/sensitivity/rotation persistence */
void touch_prefs_apply(void);
void touch_prefs_save(void);

void status_screen_reflow(void);

extern void prospector_brightness_step(int delta);
extern uint8_t prospector_brightness_get(void);
int prospector_touchpad_sens_get(void);
void prospector_touchpad_sens_step(int delta);
void prospector_touch_set_orientation(int rot);
bool prospector_touch_tap(int sx, int sy, bool hold); /* touch_input.c -> touch_nav.c */
bool prospector_touchpad_active(void);
bool prospector_touch_has_action(int sx, int sy);

/* ------------------------------- views -------------------------------- */
extern const struct view_def view_normal;
extern const struct view_def view_home;
extern const struct view_def view_settings;
extern const struct view_def view_media;
extern const struct view_def view_fkeys;
extern const struct view_def view_symbols;
extern const struct view_def view_numpad;
extern const struct view_def view_modifiers;
extern const struct view_def view_trackpad;
extern const struct view_def view_pad;
extern const struct view_def view_calc;
extern const struct view_def view_clipboard;
extern const struct view_def view_theme;
