/* Tap routing, view state, and the idle timeout for the touch UI. Owns the
 * full-screen overlay and the display-thread timer that drains taps. All per-view
 * behaviour comes from view_defs[] (touch_views.c) -- this file only routes. */

#include "touch_ui.h"

lv_obj_t *touch_overlay;
enum ui_view cur_view = VIEW_NORMAL;
int cur_page;

static atomic_t pending_tap = ATOMIC_INIT(0); /* 0 = none, else (bit20 | sx<<10 | sy) */
static int64_t last_tap_ms;

/* Weak fallbacks: the trackpad sensitivity lives in touch_input.c; get() < 0 means
 * the driver is absent, so the settings sensitivity cells are simply not drawn. */
__weak int prospector_touchpad_sens_get(void) { return -1; }
__weak void prospector_touchpad_sens_step(int delta) { ARG_UNUSED(delta); }

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

/* Portrait tap position -> the logical cell id the handlers use (identity unless
 * the view re-arranges its 2x3 grid and provides a map). */
static int logical_cell(int cell) {
    const uint8_t *map = view_defs[cur_view].portrait_map;
    if (!(ui_rot & 1) || map == NULL || cell < 0 || cell > 5) {
        return cell;
    }
    return map[cell];
}

void show_view(enum ui_view v) {
    cur_view = v;
    cur_page = 0;
    /* Views that don't keep armed one-shot mods abandon them, so an armed mod
     * can't silently apply to normal typing later (e.g. a stray Ctrl+A). */
    if (!view_defs[v].keeps_mods) {
        pending_mods = 0;
    }
    if (v == VIEW_NORMAL) {
        lv_obj_add_flag(touch_overlay, LV_OBJ_FLAG_HIDDEN);
    } else {
        build_view(v);
        lv_obj_clear_flag(touch_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

/* Runs on the LVGL/display thread (lv_timer_handler). */
static void ui_timer_cb(lv_timer_t *timer) {
    ARG_UNUSED(timer);

    int v = atomic_set(&pending_tap, 0);
    if (v & (1 << 20)) {
        int sx = (v >> 10) & 0x3FF, sy = v & 0x3FF;
        last_tap_ms = k_uptime_get();
        view_defs[cur_view].on_tap(logical_cell(cell_from_coords(sx, sy)));
    }

    if (view_defs[cur_view].idle_timeout &&
        (k_uptime_get() - last_tap_ms) > TOUCH_TIMEOUT_MS) {
        show_view(VIEW_NORMAL);
    }
}

/* Create the (hidden) full-screen overlay + the tap-drain timer. Called once from
 * zmk_display_status_screen() in status_screen.c. */
void touch_ui_attach(lv_obj_t *screen) {
    touch_overlay = lv_obj_create(screen);
    lv_obj_set_size(touch_overlay, SCR_W, SCR_H);
    lv_obj_set_pos(touch_overlay, 0, 0);
    lv_obj_set_style_bg_color(touch_overlay, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(touch_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(touch_overlay, 0, LV_PART_MAIN);
    lv_obj_add_flag(touch_overlay, LV_OBJ_FLAG_HIDDEN);

    last_tap_ms = k_uptime_get();
    lv_timer_create(ui_timer_cb, 30, NULL);
}
