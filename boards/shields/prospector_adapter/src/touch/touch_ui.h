#pragma once

/* Shared internals of the touch UI (src/touch/). One feature per .c file:
 *   tools/touch_input.c    -- CST816S gesture driver (taps + trackpad -> mouse HID)
 *   tools/touch_keys.c     -- workqueue key/macro sending (the thread-safety boundary)
 *   tools/touch_rotation.c -- 4-step display rotation (MADCTL + LVGL resolution swap)
 *   tools/touch_draw.c     -- grid geometry + button drawing
 *   tools/touch_nav.c      -- tap routing, view state, idle timeout, overlay + timer
 *   touch_main.c           -- build_view() dispatcher
 *   views/                 -- per-view renderers (one file per view)
 * ../status_screen.c -- screen assembly (widgets + touch_ui_attach)
 */

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>

#include "display_colors.h"

/* ------------------------------- constants -------------------------------- */

#define SCR_W 280
#define SCR_H 240
#define TOUCH_TIMEOUT_MS 30000
#define TOUCH_HOLD_MS 700 /* press-and-lift >= this = a hold, not a tap */
#define BRIGHTNESS_STEP 10
#define KEYS_PER_PAGE 7

/* Settings-cell limits, used to grey out a +/- control at its end stop AND as the
 * actual clamp bounds on the other side of each seam (touch_input.c aliases
 * TP_SENS_MAX to SETTINGS_SENS_MAX; brightness.c clamps to SETTINGS_BRIGHT_MIN/MAX)
 * -- single source, nothing to keep in sync by hand. */
#define SETTINGS_SENS_MAX 10
#define SETTINGS_BRIGHT_MAX 100
#define SETTINGS_BRIGHT_MIN 5

/* Trackpad scroll lane: logical coord >= this along the LONG axis (280) is the
 * scroll lane -- far-right strip in landscape, bottom strip in portrait
 * (horizontal, swipe right = scroll down). ONE constant serving both sides of
 * the seam: touch_input.c tests the gesture boundary against it, build_trackpad
 * (touch_views.c) draws the lane divider flush to it. */
#define TP_SCROLL_ZONE 240

/* Shared colour palette lives in display_colors.h (included below via touch_ui.h).
 * Touch UI roles: PRIMARY = keys, RED = back/exit, ACCENT = nav/rotate/armed,
 * GREEN = settings +, YELLOW = settings -. Surface colours also in display_colors.h. */

/* The Waveshare 1.69" glass has rounded corners, R5.15mm ~= 44px at this panel's
 * ~0.117mm/px. Chrome drawn inside the corner arcs gets physically clipped, so the
 * button grid is inset by UI_PAD and full-screen frames use GLASS_RADIUS. */
#define GLASS_RADIUS 44
#define UI_PAD 5
#define BTN_RADIUS 14


enum action_type {
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

struct page_cell {
    int row;
    int col;
    int row_span;
    int col_span;
    const char *label;
    const lv_image_dsc_t *icon;
    uint32_t color;
    enum action_type action;
    union {
        const struct view_def *view;
        uint32_t keycode;
        const char *macro;
        void (*func)(int cell);
        struct {
            void (*cb)(int val);
            int val;
        } custom;
    } arg;
};

/* Everything navigation needs to know about a view, declared not implied. */
struct view_def
{
  const struct page_cell *cells; /* declarative layout and actions; NULL-terminated */
  const struct page_cell *cells_portrait; /* optional explicit portrait layout override */
  const struct page_cell *const *pages; /* array of paginated landscape layouts */
  const struct page_cell *const *pages_portrait; /* array of paginated portrait overrides */
  uint8_t num_pages;           /* number of pages in the pages array */
  void (*build)(void);         /* renderer; NULL = nothing to draw (NORMAL) */
  bool idle_timeout;           /* return to NORMAL after TOUCH_TIMEOUT_MS idle */
  bool keeps_mods;             /* armed one-shot mods survive entering this view */
  void (*on_hold)(int cell);   /* long-press handler; NULL = holds act as taps */
  void (*on_enter)(void);      /* called once when the view is shown */
};

/* ----------------------------- shared state ------------------------------- */

extern lv_obj_t *touch_overlay; /* full-screen touch UI layer (touch_nav.c) */
extern lv_obj_t *cur_view_btns[32]; /* cached declarative button objects for mutation */
extern const struct view_def *cur_view;   /* current view (touch_nav.c) */
extern int cur_page;            /* page of the paginated key screens (touch_nav.c) */
extern uint8_t pending_mods;    /* one-shot mods, applied to the next key (touch_keys.c) */
extern int grid_rows;           /* current screen's grid (touch_draw.c) */
extern int grid_cols;
extern uint8_t ui_rot; /* 0..3 = 0/90/180/270 deg CW (touch_rotation.c) */

/* Logical screen dimensions for the current orientation. */
static inline lv_coord_t scr_w(void) { return (ui_rot & 1) ? SCR_H : SCR_W; }
static inline lv_coord_t scr_h(void) { return (ui_rot & 1) ? SCR_W : SCR_H; }

/* Optional custom HOME icons (drop converted assets in src/icons */
extern const lv_image_dsc_t icon_trackpad __weak;
extern const lv_image_dsc_t icon_modkeys __weak;
extern const lv_image_dsc_t icon_numpad __weak;
extern const lv_image_dsc_t icon_symbols __weak;
extern const lv_image_dsc_t icon_fkeys __weak;


/* ------------------------------- functions -------------------------------- */
/* touch_draw.c */
lv_obj_t *draw_cell(int row, int col, int w_cells, const char *text, uint32_t accent);
lv_obj_t *draw_cell_ext(int row, int col, int w_cells, int h_cells, const char *text, uint32_t accent, bool filled);
lv_obj_t *draw_cell_icon(int row, int col, const lv_image_dsc_t *icon, const char *fallback, uint32_t accent);
lv_obj_t *draw_cell_icon_ext(int row, int col, int w_cells, int h_cells, const lv_image_dsc_t *icon, const char *fallback, uint32_t accent);

/* touch_views.c (now in touch_main.c) */
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

/* ../status_screen.c: re-position the NORMAL screen's widgets for the current
 * orientation (called by settings_apply_rotation). */
void status_screen_reflow(void);

/* Seams to the rest of the firmware. Brightness is src/brightness.c (dongle display
 * dimmer -- never the keyboard &bl relay). The trackpad sensitivity + orientation
 * hooks are implemented by touch_input.c; weak fallbacks (sens in touch_nav.c,
 * orientation in touch_rotation.c) keep the layout linkable without it. */
extern void prospector_brightness_step(int delta);
extern uint8_t prospector_brightness_get(void);
int prospector_touchpad_sens_get(void);
void prospector_touchpad_sens_step(int delta);
void prospector_touch_set_orientation(int rot);
bool prospector_touch_tap(int sx, int sy, bool hold); /* touch_input.c -> touch_nav.c */
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
