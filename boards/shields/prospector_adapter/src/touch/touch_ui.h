#pragma once

/* Shared internals of the touch UI (src/touch/). One feature per .c file:
 *   tools/touch_input.c    -- CST816S gesture driver (taps + trackpad -> mouse HID)
 *   tools/touch_keys.c     -- workqueue key/macro sending (the thread-safety boundary)
 *   tools/touch_rotation.c -- 4-step display rotation (MADCTL + LVGL resolution swap)
 *   tools/touch_draw.c     -- grid geometry + button drawing
 *   tools/touch_nav.c      -- tap routing, view state, idle timeout, overlay + timer
 *   touch_main.c           -- view registry (view_defs[]) + build_view() dispatcher
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

/* Enum order carries no semantics -- per-view behaviour (grid, timeout, portrait
 * handling, one-shot-mod policy) is declared in view_defs[] (touch_views.c). */
enum ui_view
{
  VIEW_NORMAL,
  VIEW_HOME,
  VIEW_SETTINGS,
  VIEW_MEDIA,
  VIEW_FKEYS,
  VIEW_NUMPAD,
  VIEW_SYMBOLS,
  VIEW_MODIFIERS,
  VIEW_TRACKPAD,
  VIEW_PAD,
  VIEW_CALC,
  VIEW_COUNT,
};

/* Everything navigation needs to know about a view, declared not implied. */
struct view_def
{
  void (*build)(void);         /* renderer; NULL = nothing to draw (NORMAL) */
  void (*on_tap)(int cell);    /* tap handler (cell = logical cell id) */
  uint8_t rows, cols;          /* landscape grid */
  const uint8_t *portrait_map; /* portrait tap pos -> logical cell; NULL = identity */
  bool rearrange_2x3;          /* portrait: re-arrange the 2x3 grid to 3x2 */
  bool idle_timeout;           /* return to NORMAL after TOUCH_TIMEOUT_MS idle */
  bool keeps_mods;             /* armed one-shot mods survive entering this view */
  void (*on_hold)(int cell);   /* long-press handler; NULL = holds act as taps */
};
extern const struct view_def view_defs[VIEW_COUNT];

/* ----------------------------- shared state ------------------------------- */

extern lv_obj_t *touch_overlay; /* full-screen touch UI layer (touch_nav.c) */
extern enum ui_view cur_view;   /* current view (touch_nav.c) */
extern int cur_page;            /* page of the paginated key screens (touch_nav.c) */
extern uint8_t pending_mods;    /* one-shot mods, applied to the next key (touch_keys.c) */
extern int grid_rows;           /* current screen's grid (touch_draw.c) */
extern int grid_cols;
extern uint8_t ui_rot; /* 0..3 = 0/90/180/270 deg CW (touch_rotation.c) */

/* Logical screen dimensions for the current orientation. */
static inline lv_coord_t scr_w(void) { return (ui_rot & 1) ? SCR_H : SCR_W; }
static inline lv_coord_t scr_h(void) { return (ui_rot & 1) ? SCR_W : SCR_H; }

/* Paginated key screens: the 7 key cells of a 3x3 page (touch_draw.c). */
extern const int key_cells[KEYS_PER_PAGE];

/* Portrait re-arrangement of the 2x3 screens (touch_draw.c). */
extern const uint8_t p23_pos[6];

/* Optional custom HOME icons (drop converted assets in src/icons/, see its
 * README). Weak: with no asset file present the symbol resolves to NULL and
 * the cell falls back to its text label. */
extern const lv_image_dsc_t icon_trackpad __weak;
extern const lv_image_dsc_t icon_modkeys __weak;
extern const lv_image_dsc_t icon_numpad __weak;
extern const lv_image_dsc_t icon_symbols __weak;
extern const lv_image_dsc_t icon_fkeys __weak;

/* ------------------------------- functions -------------------------------- */

/* touch_draw.c */
void draw_cell(int row, int col, int w_cells, const char *text, uint32_t accent);
void draw_cell_l(int lcell, const char *text, uint32_t accent);
void draw_cell_on_l(int lcell, const char *text, uint32_t accent);
void draw_cell_icon(int row, int col, const lv_image_dsc_t *icon, const char *fallback,
                    uint32_t accent);
void draw_key_page(const char *const *lbls, int n, int page);

/* touch_views.c */
void build_view(enum ui_view v);

/* touch_nav.c */
void show_view(enum ui_view v);
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

/* ------------------------------- views -------------------------------- */
// NORMAL
extern void tap_normal(int cell);

// HOME
extern void build_home(void);
extern void tap_home(int cell);
extern void hold_home(int cell);

// SETTINGS
extern void build_settings(void);
extern void tap_settings(int cell);

// MEDIA
extern void build_media(void);
extern void tap_media(int cell);

// FKEYS / SYMBOLS
extern void build_fkeys(void);
extern void tap_fkeys(int cell);
extern void build_symbols(void);
extern void tap_symbols(int cell);

// NUMPAD
extern void build_numpad(void);
extern void tap_numpad(int cell);

// MODIFIERS
extern void build_modifiers(void);
extern void tap_modifiers(int cell);

// PAD
extern void build_pad(void);
extern void tap_pad(int cell);

// TRACKPAD
extern void build_trackpad(void);
extern void tap_trackpad(int cell);

// CALCULATOR
extern void build_calc(void);
extern void tap_calc(int cell);
