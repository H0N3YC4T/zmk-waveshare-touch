#include <lvgl.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>

#include "modifier_indicator.h"
#include "wpm_meter.h"
#include "layer_display.h"
#include "battery_circles.h"
#include "output.h"
#include "display_colors.h"

LOG_MODULE_REGISTER(prospector_touch_ui, LOG_LEVEL_INF);

/* dongle-only display dimmer (src/brightness.c) -- never touches the &bl relay */
extern void prospector_brightness_step(int delta);
extern uint8_t prospector_brightness_get(void);

/* Trackpad pointer-sensitivity level (0..10), implemented by the keyboard repo's
 * touch driver (touch_input.c). Weak fallbacks keep this module standalone: get()
 * < 0 means the trackpad driver is absent, so the settings sensitivity cells are
 * simply not drawn. */
__weak int prospector_touchpad_sens_get(void) { return -1; }
__weak void prospector_touchpad_sens_step(int delta) { ARG_UNUSED(delta); }
/* Told the UI orientation (0..3 = 0/90/180/270 deg CW) so touch coords rotate with
 * the image. Weak no-op keeps this module standalone. */
__weak void prospector_touch_set_orientation(int rot) { ARG_UNUSED(rot); }

static struct zmk_widget_modifier_indicator modifier_indicator_widget;
static struct zmk_widget_wpm_meter wpm_meter_widget;
static struct zmk_widget_layer_display layer_display_widget;
static struct zmk_widget_battery_circles battery_circles_widget;
static struct zmk_widget_output output_widget;

/* ------------------------------------------------------------------------- */
/* Touch navigation UI (280x240, 2x3 grid, cells 0..5).                      */
/*   Normal --tap--> HOME (3 TRACKPAD / 4 SET / 5 KEYS hub; media lives in    */
/*     the hub). Paginated 3x3 key screens: 7 keys; cell 1 = Back(page 0)/    */
/*     Prev-page, cell 7 = Next-page (blue). Numpad is 4x4.                   */
/*   Idle timeout returns HOME/SETTINGS to Normal; everything from the hub    */
/*   onwards (incl. media and the trackpad) never times out -- exit is        */
/*   always explicit.                                                         */
/* ------------------------------------------------------------------------- */

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

/* Views >= VIEW_HUB never idle-timeout (media is hub content; the trackpad is
 * entered from HOME but deliberately keeps the no-timeout behaviour). */
enum ui_view {
    VIEW_NORMAL, VIEW_HOME, VIEW_SETTINGS,
    VIEW_HUB, VIEW_MEDIA, VIEW_FKEYS, VIEW_NUMPAD, VIEW_SYMBOLS, VIEW_MODIFIERS, VIEW_TRACKPAD,
};

/* UI orientation: 0..3 = 0/90/180/270 deg CW. 0/2 = landscape 280x240, 1/3 =
 * portrait 240x280. Rotation is pure hardware scan-out (MADCTL) + an LVGL logical-
 * resolution swap -- no software rotation, no extra buffers. The glass is centred
 * in the controller RAM (y-offset 20 of 40 spare) and the vendored st7789v driver
 * re-derives the margins per orientation, so all four orientations line up. If the
 * two portraits come out swapped vs the touch on hardware, swap the middle entries
 * here AND cases 1/3 in touch_input.c's transform. */
static uint8_t ui_rot;
static const enum display_orientation rot_to_panel[4] = {
    DISPLAY_ORIENTATION_ROTATED_270, /* calibrated landscape */
    DISPLAY_ORIENTATION_ROTATED_180, /* portrait A */
    DISPLAY_ORIENTATION_ROTATED_90,  /* landscape flipped 180 */
    DISPLAY_ORIENTATION_NORMAL,      /* portrait B */
};
static inline lv_coord_t scr_w(void) { return (ui_rot & 1) ? SCR_H : SCR_W; }
static inline lv_coord_t scr_h(void) { return (ui_rot & 1) ? SCR_W : SCR_H; }

static enum ui_view cur_view = VIEW_NORMAL;
static int cur_page;
static uint8_t pending_mods;   /* one-shot, applied to the next key sent */
static lv_obj_t *overlay;
static atomic_t pending_tap = ATOMIC_INIT(0); /* 0 = none, else (bit20 | sx<<10 | sy) */
static int64_t last_tap_ms;
static int grid_rows = 2;      /* rows in the current screen's grid (2 / 3 / 4) */
static int grid_cols = 3;      /* cols in the current screen's grid (3, or 4 = numpad) */

/* 3x3 paginated layout: 7 key cells (all but 1 = nav-top and 7 = nav-bottom). */
static const int key_cells[KEYS_PER_PAGE] = {0, 2, 3, 4, 5, 6, 8};
static const uint32_t fkeys[12] = {F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12};
static const char *const fkey_lbls[12] = {"F1", "F2", "F3",  "F4",  "F5",  "F6",
                                          "F7", "F8", "F9", "F10", "F11", "F12"};

static const uint32_t symbols[32] = {
    EXCL, AT,   HASH, DLLR, PRCNT, CARET, AMPS, STAR,
    LPAR, RPAR, MINUS, UNDER, EQUAL, PLUS, LBKT, RBKT,
    LBRC, RBRC, BSLH, PIPE, SEMI, COLON, SQT, DQT,
    COMMA, DOT, LT,  GT,  FSLH, QMARK, GRAVE, TILDE,
};
static const char *const sym_lbls[32] = {
    "!", "@", "#",  "$", "%", "^", "&", "*",
    "(", ")", "-",  "_", "=", "+", "[", "]",
    "{", "}", "\\", "|", ";", ":", "'", "\"",
    ",", ".", "<",  ">", "/", "?", "`", "~",
};

/* Called from the touch workqueue thread. Stash the packed tap coords; the lv_timer
 * drains + maps them to a cell on the display thread (no LVGL off-thread). Returns
 * true = the on-screen UI owns the tap. */
bool prospector_touch_tap(int sx, int sy) {
    atomic_set(&pending_tap, (1 << 20) | ((sx & 0x3FF) << 10) | (sy & 0x3FF));
    return true;
}

/* touch_input.c streams pointer motion/clicks (instead of cell taps) while true. */
bool prospector_touchpad_active(void) {
    return cur_view == VIEW_TRACKPAD;
}

/* Map raw rendered-screen coords to a cell in the CURRENT screen's grid. */
static int cell_from_coords(int sx, int sy) {
    int cw = scr_w() / grid_cols;
    int col = cw > 0 ? sx / cw : 0;
    if (col > grid_cols - 1) { col = grid_cols - 1; }
    if (col < 0) { col = 0; }
    int rh = scr_h() / grid_rows;
    int row = rh > 0 ? sy / rh : 0;
    if (row > grid_rows - 1) { row = grid_rows - 1; }
    if (row < 0) { row = 0; }
    return row * grid_cols + col;
}

/* Key/behaviour sends are marshalled onto the system workqueue. ZMK's HID and
 * endpoint code has no cross-thread locking: zmk_behavior_invoke_binding() writes
 * the shared keyboard report on whichever thread calls it, so invoking from the
 * LVGL display thread races ZMK's own event processing and corrupts the report.
 * Note that zmk_behavior_queue_add() is not a thread hop by itself -- with wait=0
 * it runs the behavior synchronously on the caller (see app/src/behavior_queue.c).
 * The hop has to come from our own k_work_submit(); queue_add is then called
 * inside the handler, where it also serialises with any in-flight delayed macro. */
struct pending_key {
    const char *dev;
    uint32_t param1;
};
#define KEY_RING_SZ 8
static struct pending_key key_ring[KEY_RING_SZ];
static atomic_t key_ring_head = ATOMIC_INIT(0);
static atomic_t key_ring_tail = ATOMIC_INIT(0);

static void key_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    while (atomic_get(&key_ring_tail) != atomic_get(&key_ring_head)) {
        struct pending_key pk = key_ring[atomic_get(&key_ring_tail) % KEY_RING_SZ];
        struct zmk_behavior_binding b = {.behavior_dev = pk.dev, .param1 = pk.param1};
        struct zmk_behavior_binding_event ev = {.layer = 0, .position = 0x7000,
                                                .timestamp = k_uptime_get()};
        /* Runs on the system workqueue thread (this handler's context), so queue_add's
         * synchronous fast-path is safe here -- it is NOT the display thread. */
        zmk_behavior_queue_add(&ev, b, true, 0);
        zmk_behavior_queue_add(&ev, b, false, 0);
        atomic_inc(&key_ring_tail);
    }
}
static K_WORK_DEFINE(key_work, key_work_handler);

static void queue_key(const char *dev, uint32_t param1) {
    int h = atomic_get(&key_ring_head);
    key_ring[h % KEY_RING_SZ] = (struct pending_key){.dev = dev, .param1 = param1};
    atomic_set(&key_ring_head, h + 1); /* release: publishes the ring slot */
    k_work_submit(&key_work);
}

/* Send a regular key press via ZMK's &kp behaviour, applying any one-shot mod. */
static void send_key(uint32_t keycode) {
    queue_key("key_press", keycode | ((uint32_t)pending_mods << 24));
    pending_mods = 0;
}

static void fire_macro(const char *dev) {
    queue_key(dev, 0);
}

/* One button: `pct`% of its cell width (centred), rounded, charcoal fill, accent
 * border+text; `filled` inverts (solid accent, black text) for armed/on states.
 * The grid lives inside a UI_PAD safe inset so corner buttons clear the glass arcs.
 * draw_cell() is the outline 80% default; wider variants give long labels room. */
static void draw_cell_impl(int row, int col, int w_cells, const char *text, uint32_t accent,
                           int pct, bool filled) {
    lv_coord_t cwc = (scr_w() - 2 * UI_PAD) / grid_cols;
    lv_coord_t ch = (scr_h() - 2 * UI_PAD) / grid_rows;
    lv_coord_t cw = cwc * w_cells;
    lv_coord_t bw = cw * pct / 100, bh = ch * 4 / 5;

    lv_obj_t *b = lv_obj_create(overlay);
    if (b == NULL) {
        return; /* LVGL pool exhausted -- skip this cell rather than deref NULL */
    }
    lv_obj_set_size(b, bw, bh);
    lv_obj_set_pos(b, UI_PAD + col * cwc + (cw - bw) / 2, UI_PAD + row * ch + (ch - bh) / 2);
    lv_obj_set_style_bg_color(b, lv_color_hex(filled ? accent : COLOR_BTN_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(b, lv_color_hex(accent), LV_PART_MAIN);
    lv_obj_set_style_border_width(b, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(b, BTN_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_pad_all(b, 0, LV_PART_MAIN);

    lv_obj_t *l = lv_label_create(b);
    if (l == NULL) {
        return;
    }
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(l, lv_color_hex(filled ? 0x000000 : accent), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(l, 1, LV_PART_MAIN);
    lv_obj_center(l);
}

static void draw_cell(int row, int col, int w_cells, const char *text, uint32_t accent) {
    draw_cell_impl(row, col, w_cells, text, accent, 80, false);
}

/* Armed/on state: solid accent fill, black text. */
static void draw_cell_on(int row, int col, const char *text, uint32_t accent) {
    draw_cell_impl(row, col, 1, text, accent, 80, true);
}

/* 2x3-logical cell placement for both orientations. In portrait the six cells
 * re-arrange to 3 rows x 2 cols preserving pair order: top outer pair first, then
 * back beside the centre action, then the bottom outer pair. Same table maps a
 * portrait tap back to the logical cell id the handlers use. HOME overrides this
 * with row spans (see build_view). */
static const uint8_t p23_pos[6] = {0, 2, 1, 4, 3, 5}; /* portrait pos -> logical cell */
static void draw_cell_l_impl(int lcell, const char *text, uint32_t accent, bool filled) {
    if (ui_rot & 1) {
        for (int p = 0; p < 6; p++) {
            if (p23_pos[p] == lcell) {
                draw_cell_impl(p / 2, p % 2, 1, text, accent, 80, filled);
                return;
            }
        }
    } else {
        draw_cell_impl(lcell / 3, lcell % 3, 1, text, accent, 80, filled);
    }
}
static void draw_cell_l(int lcell, const char *text, uint32_t accent) {
    draw_cell_l_impl(lcell, text, accent, false);
}
static void draw_cell_on_l(int lcell, const char *text, uint32_t accent) {
    draw_cell_l_impl(lcell, text, accent, true);
}

/* Draw one page of a paginated 3x3 key screen: 7 keys in cells 0,2,3,4,5,6,8;
 * cell 1 (top-middle) = Back/Prev, cell 7 (bottom-middle) = Next. */
static void draw_key_page(const char *const *lbls, int n, int page) {
    int pages = (n + KEYS_PER_PAGE - 1) / KEYS_PER_PAGE;
    for (int i = 0; i < KEYS_PER_PAGE; i++) {
        int idx = page * KEYS_PER_PAGE + i;
        if (idx < n) {
            draw_cell(key_cells[i] / 3, key_cells[i] % 3, 1, lbls[idx], COLOR_ACCENT);
        }
    }
    if (page == 0) {
        draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_BACK);  /* Back to hub */
    } else {
        draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_PAGE);    /* Prev page */
    }
    if (pages > 1) {
        draw_cell(2, 1, 1, LV_SYMBOL_DOWN, COLOR_PAGE);  /* Next page (cell 7) */
    }
}

static void build_view(enum ui_view v) {
    lv_obj_clean(overlay);
    if (v == VIEW_NUMPAD) {
        grid_rows = 4; grid_cols = 4; /* square-ish grids keep their layout in portrait */
    } else if (v == VIEW_FKEYS || v == VIEW_SYMBOLS || v == VIEW_SETTINGS) {
        grid_rows = 3; grid_cols = 3;
    } else if (ui_rot & 1) {
        grid_rows = 3; grid_cols = 2; /* 2x3 views re-arrange to 3x2 in portrait */
    } else {
        grid_rows = 2; grid_cols = 3;
    }

    switch (v) {
    case VIEW_HOME: /* icons: back / trackpad / settings / keys. Portrait: back spans
                     * the top row, trackpad|settings middle, keys spans the bottom
                     * (touch remap in logical_cell()). */
        if (ui_rot & 1) {
            draw_cell(0, 0, 2, LV_SYMBOL_UP, COLOR_BACK);
            draw_cell(1, 0, 1, LV_SYMBOL_GPS, COLOR_ACCENT);
            draw_cell(1, 1, 1, LV_SYMBOL_SETTINGS, COLOR_ACCENT);
            draw_cell(2, 0, 2, LV_SYMBOL_KEYBOARD, COLOR_ACCENT);
        } else {
            draw_cell(0, 0, 3, LV_SYMBOL_UP, COLOR_BACK);
            draw_cell(1, 0, 1, LV_SYMBOL_GPS, COLOR_ACCENT);
            draw_cell(1, 1, 1, LV_SYMBOL_SETTINGS, COLOR_ACCENT);
            draw_cell(1, 2, 1, LV_SYMBOL_KEYBOARD, COLOR_ACCENT);
        }
        break;
    case VIEW_MEDIA: /* logical cells; portrait pairs vol-|vol+ / back|play / prev|next */
        draw_cell_l(0, LV_SYMBOL_VOLUME_MID, COLOR_ACCENT);
        draw_cell_l(1, LV_SYMBOL_UP, COLOR_BACK);
        draw_cell_l(2, LV_SYMBOL_VOLUME_MAX, COLOR_ACCENT);
        draw_cell_l(3, LV_SYMBOL_PREV, COLOR_ACCENT);
        draw_cell_l(4, LV_SYMBOL_PLAY, COLOR_ACCENT);
        draw_cell_l(5, LV_SYMBOL_NEXT, COLOR_ACCENT);
        break;
    case VIEW_SETTINGS: {
        /* 3x3. One setting per outer column: left = trackpad sensitivity, right =
         * display brightness. Rows 0/1 = + and - (greyed once the value is at its
         * end stop); row 2 = icon + live-value readouts (blue, not keys). Middle
         * column: back / rotate / empty. Cells 0-5 keep the 2x3 handler numbering.
         * (No sun glyph exists in LVGL's built-in set; eye-open marks brightness.) */
        char lbl[20];
        int br = prospector_brightness_get();
        int sn = prospector_touchpad_sens_get();
        if (sn >= 0) {
            draw_cell(0, 0, 1, LV_SYMBOL_PLUS,
                      sn >= SETTINGS_SENS_MAX ? COLOR_HINT_GLYPH : COLOR_ACCENT);
            draw_cell(1, 0, 1, LV_SYMBOL_MINUS, sn <= 0 ? COLOR_HINT_GLYPH : COLOR_ACCENT);
            lv_snprintf(lbl, sizeof(lbl), LV_SYMBOL_GPS " %d", sn);
            draw_cell(2, 0, 1, lbl, COLOR_PAGE);
        }
        draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_BACK);
        draw_cell(1, 1, 1, LV_SYMBOL_REFRESH, COLOR_PAGE); /* rotate the display 180deg */
        draw_cell(0, 2, 1, LV_SYMBOL_PLUS,
                  br >= SETTINGS_BRIGHT_MAX ? COLOR_HINT_GLYPH : COLOR_ACCENT);
        draw_cell(1, 2, 1, LV_SYMBOL_MINUS,
                  br <= SETTINGS_BRIGHT_MIN ? COLOR_HINT_GLYPH : COLOR_ACCENT);
        lv_snprintf(lbl, sizeof(lbl), LV_SYMBOL_EYE_OPEN " %d%%", br);
        draw_cell(2, 2, 1, lbl, COLOR_PAGE);
        break;
    }
    case VIEW_HUB: /* hub: 0 F-keys, 1 Back, 2 numpad, 3 symbols, 4 media, 5 mods.
                    * No LVGL glyphs exist for F-keys/modifiers -> short text. */
        draw_cell_l(0, "Fn", COLOR_ACCENT);
        draw_cell_l(1, LV_SYMBOL_UP, COLOR_BACK);
        draw_cell_l(2, "123", COLOR_ACCENT);
        draw_cell_l(3, "#$%", COLOR_ACCENT);
        draw_cell_l(4, LV_SYMBOL_AUDIO, COLOR_ACCENT);
        draw_cell_l(5, "MOD", COLOR_ACCENT);
        break;
    case VIEW_FKEYS:
        draw_key_page(fkey_lbls, 12, cur_page);
        break;
    case VIEW_NUMPAD: /* 4x4 calc grid; ops (blue) in col 3; cell 12 = Back, 14 = Enter */
        draw_cell(0, 0, 1, "7", COLOR_ACCENT);
        draw_cell(0, 1, 1, "8", COLOR_ACCENT);
        draw_cell(0, 2, 1, "9", COLOR_ACCENT);
        draw_cell(0, 3, 1, "+", COLOR_PAGE);
        draw_cell(1, 0, 1, "4", COLOR_ACCENT);
        draw_cell(1, 1, 1, "5", COLOR_ACCENT);
        draw_cell(1, 2, 1, "6", COLOR_ACCENT);
        draw_cell(1, 3, 1, "-", COLOR_PAGE);
        draw_cell(2, 0, 1, "1", COLOR_ACCENT);
        draw_cell(2, 1, 1, "2", COLOR_ACCENT);
        draw_cell(2, 2, 1, "3", COLOR_ACCENT);
        draw_cell(2, 3, 1, "*", COLOR_PAGE);
        draw_cell(3, 0, 1, LV_SYMBOL_UP, COLOR_BACK);
        draw_cell(3, 1, 1, "0", COLOR_ACCENT);
        draw_cell(3, 2, 1, LV_SYMBOL_NEW_LINE, COLOR_PAGE);
        draw_cell(3, 3, 1, "/", COLOR_PAGE);
        break;
    case VIEW_SYMBOLS:
        draw_key_page(sym_lbls, 32, cur_page);
        break;
    case VIEW_MODIFIERS: /* one-shot mods; armed = solid blue fill, inactive = outline */
        if (pending_mods & MOD_LCTL) { draw_cell_on_l(0, "CTRL", COLOR_PAGE); }
        else                         { draw_cell_l(0, "CTRL", COLOR_ACCENT); }
        draw_cell_l(1, LV_SYMBOL_UP, COLOR_BACK);
        if (pending_mods & MOD_LSFT) { draw_cell_on_l(2, "SHFT", COLOR_PAGE); }
        else                         { draw_cell_l(2, "SHFT", COLOR_ACCENT); }
        if (pending_mods & MOD_LALT) { draw_cell_on_l(3, "ALT", COLOR_PAGE); }
        else                         { draw_cell_l(3, "ALT", COLOR_ACCENT); }
        if (pending_mods & MOD_LGUI) { draw_cell_on_l(5, "GUI", COLOR_PAGE); }
        else                         { draw_cell_l(5, "GUI", COLOR_ACCENT); }
        break;
    case VIEW_TRACKPAD: {
        /* Whole screen = trackpad (gestures in touch_input.c): drag = move pointer,
         * 1 tap = L-click, 2 taps = R-click, top-left corner = exit. Scroll lane =
         * logical coord >= 240 along the long axis, drawn flush to the edge:
         * right-side vertical strip in landscape, bottom horizontal strip in
         * portrait (swipe right = scroll down). */
        lv_obj_t *lane = lv_obj_create(overlay);
        if (lane != NULL) {
            if (ui_rot & 1) {
                lv_obj_set_size(lane, scr_w(), scr_h() - 240);
                lv_obj_set_pos(lane, 0, 240);
            } else {
                lv_obj_set_size(lane, scr_w() - 240, scr_h());
                lv_obj_set_pos(lane, 240, 0);
            }
            lv_obj_set_style_bg_color(lane, lv_color_hex(COLOR_LANE_BG), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(lane, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_color(lane, lv_color_hex(COLOR_LANE_EDGE), LV_PART_MAIN);
            lv_obj_set_style_border_width(lane, 1, LV_PART_MAIN);
            lv_obj_set_style_radius(lane, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(lane, 0, LV_PART_MAIN);
        }
        lv_obj_t *ex = lv_label_create(overlay);
        if (ex != NULL) {
            lv_label_set_text(ex, LV_SYMBOL_CLOSE); /* X: exit the pad, not a nav level */
            lv_obj_set_style_text_font(ex, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(ex, lv_color_hex(COLOR_BACK), LV_PART_MAIN);
            lv_obj_set_pos(ex, 16, 12); /* corner-exit affordance, clear of the glass arc */
        }
        lv_obj_t *hint = lv_label_create(overlay);
        if (hint != NULL) {
            lv_label_set_text(hint, "TRACKPAD");
            lv_obj_set_style_text_font(hint, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(hint, lv_color_hex(COLOR_HINT), LV_PART_MAIN);
            /* centred on the pad area (excluding the lane) */
            if (ui_rot & 1) {
                lv_obj_align(hint, LV_ALIGN_CENTER, 0, -20);
            } else {
                lv_obj_align(hint, LV_ALIGN_CENTER, -20, 0);
            }
        }
        break;
    }
    default:
        break;
    }

    /* Armed one-shot modifier -> blue frame visible from any key page. Drawn as its
     * own rounded-rect child (radius = glass) so it follows the screen's physical
     * corners. Cleaned up with the rest of the view. */
    if (pending_mods) {
        lv_obj_t *frame = lv_obj_create(overlay);
        if (frame != NULL) {
            lv_obj_set_size(frame, scr_w(), scr_h());
            lv_obj_set_pos(frame, 0, 0);
            lv_obj_set_style_bg_opa(frame, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_color(frame, lv_color_hex(COLOR_PAGE), LV_PART_MAIN);
            lv_obj_set_style_border_width(frame, 3, LV_PART_MAIN);
            lv_obj_set_style_radius(frame, GLASS_RADIUS, LV_PART_MAIN);
            lv_obj_set_style_pad_all(frame, 0, LV_PART_MAIN);
        }
    }
}

static void show_view(enum ui_view v) {
    cur_view = v;
    cur_page = 0;
    /* Leaving the macro hub abandons any armed one-shot modifier, so it can't
     * silently apply to normal typing later (e.g. armed CTRL -> a stray Ctrl+A). */
    if (v < VIEW_HUB) {
        pending_mods = 0;
    }
    if (v == VIEW_NORMAL) {
        lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    } else {
        build_view(v);
        lv_obj_clear_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

/* Nav + key handling shared by all paginated key screens. */
static void handle_key_page(const uint32_t *keys, int n, int cell) {
    int pages = (n + KEYS_PER_PAGE - 1) / KEYS_PER_PAGE;
    if (cell == 1) {
        if (cur_page == 0) {
            show_view(VIEW_HUB);
        } else {
            cur_page--;
            build_view(cur_view);
        }
        return;
    }
    if (cell == 7) {
        if (pages > 1) {
            cur_page = (cur_page + 1) % pages;
            build_view(cur_view);
        }
        return;
    }
    for (int i = 0; i < KEYS_PER_PAGE; i++) {
        if (key_cells[i] == cell) {
            int idx = cur_page * KEYS_PER_PAGE + i;
            if (idx < n) {
                send_key(keys[idx]);
            }
            return;
        }
    }
}

/* Rotate the UI 90deg CW per tap (settings screen). Pure hardware scan-out change
 * (MADCTL via display_set_orientation) + an LVGL logical-resolution swap; the touch
 * driver is told the orientation, and the current view is rebuilt for the new
 * dimensions. NOTE: the NORMAL status screen's widgets are laid out for landscape
 * and are NOT reflowed yet -- in portrait they render clipped. The touch UI itself
 * is fully portrait-aware. */
static void settings_apply_rotation(void) {
    const struct device *disp = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(disp)) {
        return;
    }
    display_set_orientation(disp, rot_to_panel[ui_rot]);
    lv_display_set_resolution(lv_display_get_default(), scr_w(), scr_h());
    lv_obj_set_size(overlay, scr_w(), scr_h());
    prospector_touch_set_orientation(ui_rot);
    build_view(cur_view); /* re-lay the current screen for the new dimensions */
    lv_obj_invalidate(lv_screen_active());
}

static void handle_tap(int cell) {
    last_tap_ms = k_uptime_get();

    switch (cur_view) {
    case VIEW_NORMAL:
        show_view(VIEW_HOME);
        break;
    case VIEW_HOME:
        if (cell <= 2) {
            show_view(VIEW_NORMAL);
        } else if (cell == 3) {
            show_view(VIEW_TRACKPAD);
        } else if (cell == 4) {
            show_view(VIEW_SETTINGS);
        } else {
            show_view(VIEW_HUB);
        }
        break;
    case VIEW_MEDIA:
        switch (cell) {
        case 0: fire_macro("touch_macro_0"); break;
        case 1: show_view(VIEW_HUB); break; /* media is reached from the hub now */
        case 2: fire_macro("touch_macro_2"); break;
        case 3: fire_macro("touch_macro_4"); break;
        case 4: fire_macro("touch_macro_3"); break;
        case 5: fire_macro("touch_macro_5"); break;
        default: break;
        }
        break;
    case VIEW_SETTINGS: /* 3x3; cells 6-8 are the readout row -> no-ops via default */
        switch (cell) {
        case 0: prospector_touchpad_sens_step(+1); build_view(VIEW_SETTINGS); break;
        case 1: show_view(VIEW_HOME); break;
        case 2: prospector_brightness_step(+BRIGHTNESS_STEP); build_view(VIEW_SETTINGS); break;
        case 3: prospector_touchpad_sens_step(-1); build_view(VIEW_SETTINGS); break;
        case 4: ui_rot = (ui_rot + 1) & 3; settings_apply_rotation(); break; /* +90deg CW */
        case 5: prospector_brightness_step(-BRIGHTNESS_STEP); build_view(VIEW_SETTINGS); break;
        default: break;
        }
        break;
    case VIEW_HUB:
        switch (cell) {
        case 0: show_view(VIEW_FKEYS); break;
        case 1: show_view(VIEW_HOME); break;
        case 2: show_view(VIEW_NUMPAD); break;
        case 3: show_view(VIEW_SYMBOLS); break;
        case 4: show_view(VIEW_MEDIA); break;
        case 5: show_view(VIEW_MODIFIERS); break;
        default: break;
        }
        break;
    case VIEW_FKEYS:
        handle_key_page(fkeys, 12, cell);
        break;
    case VIEW_SYMBOLS:
        handle_key_page(symbols, 32, cell);
        break;
    case VIEW_NUMPAD: {
        /* 4x4; col 3 = operators. Index 12 (Back) is 0 and handled first, so the
         * np[cell] guard only skips it -- every real key code is non-zero. */
        static const uint32_t np[16] = {N7, N8, N9, PLUS,  N4, N5, N6, MINUS,
                                        N1, N2, N3, STAR,  0,  N0, RET, FSLH};
        if (cell == 12) {
            show_view(VIEW_HUB);
        } else if (cell >= 0 && cell <= 15 && np[cell]) {
            send_key(np[cell]);
        }
        break;
    }
    case VIEW_MODIFIERS:
        switch (cell) {
        case 0: pending_mods ^= MOD_LCTL; build_view(VIEW_MODIFIERS); break;
        case 1: show_view(VIEW_HUB); break;
        case 2: pending_mods ^= MOD_LSFT; build_view(VIEW_MODIFIERS); break;
        case 3: pending_mods ^= MOD_LALT; build_view(VIEW_MODIFIERS); break;
        case 5: pending_mods ^= MOD_LGUI; build_view(VIEW_MODIFIERS); break;
        default: break;
        }
        break;
    case VIEW_TRACKPAD:
        /* touch_input.c only forwards the corner-exit tap here; the trackpad is
         * entered from HOME now, so leave back to HOME. */
        show_view(VIEW_HOME);
        break;
    default:
        break;
    }
}

/* Portrait 3x2 tap -> the 2x3 logical cell id the handlers use. Square grids,
 * settings, and the trackpad are identity (their layouts don't re-arrange). */
static int logical_cell(int cell) {
    if (!(ui_rot & 1) || cell < 0 || cell > 5) {
        return cell;
    }
    switch (cur_view) {
    case VIEW_HOME: { /* back spans row 0, keys spans row 2 */
        static const uint8_t m[6] = {0, 0, 3, 4, 5, 5};
        return m[cell];
    }
    case VIEW_MEDIA:
    case VIEW_HUB:
    case VIEW_MODIFIERS:
        return p23_pos[cell];
    default:
        return cell;
    }
}

/* Runs on the LVGL/display thread (lv_timer_handler). */
static void ui_timer_cb(lv_timer_t *timer) {
    ARG_UNUSED(timer);

    int v = atomic_set(&pending_tap, 0);
    if (v & (1 << 20)) {
        int sx = (v >> 10) & 0x3FF, sy = v & 0x3FF;
        handle_tap(logical_cell(cell_from_coords(sx, sy)));
    }

    /* Idle timeout returns to Normal -- but NOT while inside the macro hub. */
    if (cur_view != VIEW_NORMAL && cur_view < VIEW_HUB &&
        (k_uptime_get() - last_tap_ms) > TOUCH_TIMEOUT_MS) {
        show_view(VIEW_NORMAL);
    }
}

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, 255, LV_PART_MAIN);

    zmk_widget_modifier_indicator_init(&modifier_indicator_widget, screen);
    lv_obj_set_pos(zmk_widget_modifier_indicator_obj(&modifier_indicator_widget), 25, 8);

    zmk_widget_wpm_meter_init(&wpm_meter_widget, screen);
    lv_obj_set_pos(zmk_widget_wpm_meter_obj(&wpm_meter_widget), 10, 42);

    zmk_widget_layer_display_init(&layer_display_widget, screen);
    lv_obj_set_pos(zmk_widget_layer_display_obj(&layer_display_widget), 10, 142);

    zmk_widget_battery_circles_init(&battery_circles_widget, screen);
    lv_obj_set_pos(zmk_widget_battery_circles_obj(&battery_circles_widget), 11, 170);

    zmk_widget_output_init(&output_widget, screen);
    lv_obj_set_pos(zmk_widget_output_obj(&output_widget), 148, 170);

    overlay = lv_obj_create(screen);
    lv_obj_set_size(overlay, SCR_W, SCR_H);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(overlay, 0, LV_PART_MAIN);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);

    last_tap_ms = k_uptime_get();
    lv_timer_create(ui_timer_cb, 30, NULL);

    return screen;
}
