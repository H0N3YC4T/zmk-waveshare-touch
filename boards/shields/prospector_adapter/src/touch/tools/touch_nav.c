/* Tap routing, view state, and the idle timeout for the touch UI. Owns the
 * full-screen overlay and the display-thread timer that drains taps. All per-view
 * behaviour comes from view_defs[] (touch_main.c) -- this file only routes. */

#include "../touch_ui.h"

lv_obj_t *touch_overlay;
const struct view_def *cur_view = &view_normal;
int cur_page = 0;

static atomic_t pending_tap = ATOMIC_INIT(0); /* 0 = none, else (bit20 | hold<<21 | sx<<10 | sy) */
static int64_t last_tap_ms;

/* Weak fallbacks: the trackpad sensitivity lives in touch_input.c; get() < 0 means
 * the driver is absent, so the settings sensitivity cells are simply not drawn. */
__weak int prospector_touchpad_sens_get(void) { return -1; }
__weak void prospector_touchpad_sens_step(int delta) { ARG_UNUSED(delta); }

bool prospector_touchpad_active(void) {
    return cur_view == &view_trackpad;
}

/* Called from the touch workqueue thread. Stash the packed tap coords; the lv_timer
 * drains + maps them to a cell on the display thread (no LVGL off-thread). Returns
 * true = the on-screen UI owns the tap. */
bool prospector_touch_tap(int sx, int sy, bool hold) {
    atomic_set(&pending_tap,
               (1 << 20) | (hold ? (1 << 21) : 0) | ((sx & 0x3FF) << 10) | (sy & 0x3FF));
    return true;
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

bool prospector_touch_has_action(int sx, int sy) {
    int cell = cell_from_coords(sx, sy);
    return ui_has_action(cell);
}



void show_view(const struct view_def *v) {
    cur_view = v;
    cur_page = 0;
    /* Views that don't keep armed one-shot mods abandon them, so an armed mod
     * can't silently apply to normal typing later (e.g. a stray Ctrl+A). */
    if (!v->keeps_mods) {
        pending_mods = 0;
    }
    if (v == &view_normal) {
        grid_rows = 1;
        grid_cols = 1;
        lv_obj_add_flag(touch_overlay, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (v->on_enter) v->on_enter();
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
        int cell = cell_from_coords(sx, sy);
        last_tap_ms = k_uptime_get();
        
        void (*on_hold)(int) = cur_view->on_hold;
        if ((v & (1 << 21)) && on_hold != NULL) {
            on_hold(cell);
        } else {
            tap_declarative(cell);
        }
    }

    if (cur_view->idle_timeout &&
        (k_uptime_get() - last_tap_ms) > TOUCH_TIMEOUT_MS) {
        show_view(&view_normal);
    }
}

/* Create the (hidden) full-screen overlay + the tap-drain timer. Called once from
 * zmk_display_status_screen() in status_screen.c. */
void touch_ui_attach(lv_obj_t *screen) {
    touch_overlay = lv_obj_create(screen);
    lv_obj_set_size(touch_overlay, SCR_W, SCR_H);
    lv_obj_set_pos(touch_overlay, 0, 0);
    lv_obj_set_style_bg_color(touch_overlay, lv_color_hex(COLOR_BACKGROUND), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(touch_overlay, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(touch_overlay, 0, LV_PART_MAIN);
    lv_obj_add_flag(touch_overlay, LV_OBJ_FLAG_HIDDEN);

    last_tap_ms = k_uptime_get();
    lv_timer_create(ui_timer_cb, 30, NULL);
}
