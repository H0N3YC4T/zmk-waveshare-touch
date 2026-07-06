#pragma once

/* Shared internals of the touch UI (src/touch/). One feature per .c file:
 *   touch_input.c    -- CST816S gesture driver (taps + trackpad -> mouse HID)
 *   touch_keys.c     -- workqueue key/macro sending (the thread-safety boundary)
 *   touch_rotation.c -- 4-step display rotation (MADCTL + LVGL resolution swap)
 *   touch_draw.c     -- grid geometry + button drawing
 *   touch_views.c    -- per-view renderers (build_view)
 *   touch_nav.c      -- tap routing, view state, idle timeout, overlay + timer
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
#define BRIGHTNESS_STEP 10
#define KEYS_PER_PAGE 7

/* Settings-cell limits, used to grey out a +/- control at its end stop. Must track
 * the touch driver (TP_SENS_MAX) and brightness.c (5..100% clamp). */
#define SETTINGS_SENS_MAX   10
#define SETTINGS_BRIGHT_MAX 100
#define SETTINGS_BRIGHT_MIN 5

#define COLOR_ACCENT DISPLAY_COLOR_WPM_TEXT         /* lilac/purple */
#define COLOR_BACK   DISPLAY_COLOR_BATTERY_LOW_FILL /* low-battery red */
#define COLOR_PAGE   DISPLAY_COLOR_BATTERY_FILL     /* pastel battery-blue */
#define COLOR_BTN_BG 0x101216                       /* soft charcoal button fill */
#define COLOR_HINT      0x303030                    /* dim legend/hint text */
#define COLOR_HINT_GLYPH 0x505050                   /* slightly brighter hint glyphs */
#define COLOR_LANE_BG   0x0b0d10                    /* scroll-track fill (below button fill) */
#define COLOR_LANE_EDGE 0x2e3238                    /* scroll-track outline */

/* The Waveshare 1.69" glass has rounded corners, R5.15mm ~= 44px at this panel's
 * ~0.117mm/px. Chrome drawn inside the corner arcs gets physically clipped, so the
 * button grid is inset by UI_PAD and full-screen frames use GLASS_RADIUS. */
#define GLASS_RADIUS 44
#define UI_PAD 5
#define BTN_RADIUS 14

/* Enum order carries no semantics -- per-view behaviour (grid, timeout, portrait
 * handling, one-shot-mod policy) is declared in view_defs[] (touch_views.c). */
enum ui_view {
    VIEW_NORMAL, VIEW_HOME, VIEW_SETTINGS,
    VIEW_HUB, VIEW_MEDIA, VIEW_FKEYS, VIEW_NUMPAD, VIEW_SYMBOLS, VIEW_MODIFIERS, VIEW_TRACKPAD,
    VIEW_COUNT,
};

/* Everything navigation needs to know about a view, declared not implied. */
struct view_def {
    void (*build)(void);         /* renderer; NULL = nothing to draw (NORMAL) */
    void (*on_tap)(int cell);    /* tap handler (cell = logical cell id) */
    uint8_t rows, cols;          /* landscape grid */
    const uint8_t *portrait_map; /* portrait tap pos -> logical cell; NULL = identity */
    bool rearrange_2x3;          /* portrait: re-arrange the 2x3 grid to 3x2 */
    bool idle_timeout;           /* return to NORMAL after TOUCH_TIMEOUT_MS idle */
    bool keeps_mods;             /* armed one-shot mods survive entering this view */
};
extern const struct view_def view_defs[VIEW_COUNT];

/* ----------------------------- shared state ------------------------------- */

extern lv_obj_t *touch_overlay;      /* full-screen touch UI layer (touch_nav.c) */
extern enum ui_view cur_view;        /* current view (touch_nav.c) */
extern int cur_page;                 /* page of the paginated key screens (touch_nav.c) */
extern uint8_t pending_mods;         /* one-shot mods, applied to the next key (touch_keys.c) */
extern int grid_rows;                /* current screen's grid (touch_draw.c) */
extern int grid_cols;
extern uint8_t ui_rot;               /* 0..3 = 0/90/180/270 deg CW (touch_rotation.c) */

/* Logical screen dimensions for the current orientation. */
static inline lv_coord_t scr_w(void) { return (ui_rot & 1) ? SCR_H : SCR_W; }
static inline lv_coord_t scr_h(void) { return (ui_rot & 1) ? SCR_W : SCR_H; }

/* Paginated key screens: the 7 key cells of a 3x3 page (touch_draw.c). */
extern const int key_cells[KEYS_PER_PAGE];

/* Portrait re-arrangement of the 2x3 screens (touch_draw.c). */
extern const uint8_t p23_pos[6];

/* ------------------------------- functions -------------------------------- */

/* touch_draw.c */
void draw_cell(int row, int col, int w_cells, const char *text, uint32_t accent);
void draw_cell_on(int row, int col, const char *text, uint32_t accent);
void draw_cell_l(int lcell, const char *text, uint32_t accent);
void draw_cell_on_l(int lcell, const char *text, uint32_t accent);
void draw_key_page(const char *const *lbls, int n, int page);

/* touch_views.c */
void build_view(enum ui_view v);

/* touch_nav.c */
void show_view(enum ui_view v);
void touch_ui_attach(lv_obj_t *screen); /* create the overlay + the drain timer */

/* touch_keys.c */
void send_key(uint32_t keycode);
void fire_macro(const char *dev);

/* touch_rotation.c */
void settings_apply_rotation(void);

/* Seams to the rest of the firmware. Brightness is src/brightness.c (dongle display
 * dimmer -- never the keyboard &bl relay). The trackpad sensitivity + orientation
 * hooks are implemented by touch_input.c; weak fallbacks (sens in touch_nav.c,
 * orientation in touch_rotation.c) keep the layout linkable without it. */
extern void prospector_brightness_step(int delta);
extern uint8_t prospector_brightness_get(void);
int prospector_touchpad_sens_get(void);
void prospector_touchpad_sens_step(int delta);
void prospector_touch_set_orientation(int rot);
