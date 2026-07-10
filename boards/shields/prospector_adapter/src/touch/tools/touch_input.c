/*
 * SPDX-License-Identifier: MIT
 *
 * CST816S touch input for the XIAO nRF52840 + Waveshare 1.69" prospector dongle.
 */
#include <zephyr/kernel.h>

#if DT_HAS_COMPAT_STATUS_OKAY(hynitron_cst816s)

#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zmk/behavior.h>

#include "../touch_ui.h"

LOG_MODULE_REGISTER(mk1_touch, LOG_LEVEL_INF);

#if IS_ENABLED(CONFIG_ZMK_POINTING)
#include <zmk/hid.h>
#include <zmk/endpoints.h>
#include <dt-bindings/zmk/pointing.h>

#define TP_MOVE_DEADZONE_PX 8
#define TP_DTAP_MS 180            /* 2nd tap within this of the 1st = right click */
#define TP_SCROLL_PX 6


#define TP_SENS_MAX SETTINGS_SENS_MAX
#define TP_SENS_DEFAULT 5
#define TP_SENS_MULT256 205   /* per level; 5*205 = 1025/256 ~= 4.0x */
#endif

/* Waveshare 1.69" panel, as the CST816S reports it (portrait). */
#define PANEL_W 240
#define PANEL_H 280
#define TOUCH_TAP_MAX_TRAVEL 24


#define SCREEN_W 280
#define SCREEN_H 240

/* rot 0 = the hardware-calibrated baseline; if the portraits come out swapped on
 * hardware, swap cases 1/3 here AND rot_to_panel[]'s middle entries. */
static uint8_t tp_rot;
void prospector_touch_set_orientation(int rot) { tp_rot = (uint8_t)(rot & 3); }
static inline int32_t panel_to_screen_x(int32_t tx, int32_t ty) {
    switch (tp_rot) {
    case 1: return PANEL_W - tx;      /* portrait A */
    case 2: return PANEL_H - 1 - ty;  /* landscape, flipped 180 */
    case 3: return tx;                /* portrait B */
    default: return ty;               /* calibrated landscape */
    }
}
static inline int32_t panel_to_screen_y(int32_t tx, int32_t ty) {
    switch (tp_rot) {
    case 1: return PANEL_H - 1 - ty;
    case 2: return tx;
    case 3: return ty;
    default: return PANEL_W - tx;
    }
}


static int32_t cur_x, cur_y, start_x, start_y;
static bool active;
static int64_t down_ms;    /* uptime at touch-down, for hold detection */
static int32_t pending_sx, pending_sy;
static bool pending_hold;  /* the pending tap was a long-press */

static inline int32_t iabs32(int32_t v) { return v < 0 ? -v : v; }

/* Behaviors/HID must run on a workqueue, never in the input callback (driver ctx). */
static void touch_fire(struct k_work *work) {
    ARG_UNUSED(work);
    prospector_touch_tap(pending_sx, pending_sy, pending_hold);
}
static K_WORK_DEFINE(touch_work, touch_fire);

#if IS_ENABLED(CONFIG_ZMK_POINTING)
enum tp_mode { TP_PENDING, TP_MOTION, TP_SCROLL, TP_DRAG };
static atomic_t tp_click = ATOMIC_INIT(0);    /* 0 / MB1 / MB2: one-shot click pulse */
static atomic_t tp_drag_cmd = ATOMIC_INIT(0); /* 0 none / 1 press-and-hold MB1 / 2 release it */
static atomic_t tp_scroll = ATOMIC_INIT(0); /* signed vertical wheel ticks pending */
static atomic_t tp_dx = ATOMIC_INIT(0);     /* accumulated pointer deltas (raw px) */
static atomic_t tp_dy = ATOMIC_INIT(0);
static int32_t tp_start_sx, tp_start_sy;    /* touch-down screen coords */
static int32_t tp_prev_sx, tp_prev_sy;      /* last sampled point while streaming motion */
static int32_t tp_scroll_ref;               /* scroll-axis coord of the last emitted tick */
static enum tp_mode tp_mode;                /* this touch: pending / moving / scrolling / drag */
static bool tp_scroll_zone;                 /* touch started in the scroll lane */
static bool tp_first_tap;                   /* a first tap is awaiting a possible second */

static bool tp_drag_candidate;
static int32_t tp_carry_x, tp_carry_y;      /* x256 sub-pixel remainders (sens scaling) */
static uint8_t tp_sens_level = TP_SENS_DEFAULT;


int prospector_touchpad_sens_get(void) {
    return tp_sens_level;
}
void prospector_touchpad_sens_step(int delta) {
    int l = (int)tp_sens_level + delta;
    if (l < 0) { l = 0; }
    if (l > TP_SENS_MAX) { l = TP_SENS_MAX; }
    tp_sens_level = (uint8_t)l;
}

static void tp_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int dx = atomic_set(&tp_dx, 0); /* already sensitivity-scaled at accumulation time */
    int dy = atomic_set(&tp_dy, 0);
    int scroll = atomic_set(&tp_scroll, 0);
    int click = atomic_set(&tp_click, 0);
    int drag_cmd = atomic_set(&tp_drag_cmd, 0);
    if (drag_cmd == 1) {
        zmk_hid_mouse_buttons_press(MB1);
        zmk_endpoint_send_mouse_report();
    } else if (drag_cmd == 2) {
        zmk_hid_mouse_buttons_release(MB1);
        zmk_endpoint_send_mouse_report();
    }
    if (dx || dy) {
        zmk_hid_mouse_movement_set((int16_t)dx, (int16_t)dy);
        zmk_endpoint_send_mouse_report();
        zmk_hid_mouse_movement_set(0, 0);
    }
    if (scroll) {
        zmk_hid_mouse_scroll_set(0, (int16_t)scroll);
        zmk_endpoint_send_mouse_report();
        zmk_hid_mouse_scroll_set(0, 0);
    }
    if (click) {
        zmk_hid_mouse_buttons_press((zmk_mouse_button_flags_t)click);
        zmk_endpoint_send_mouse_report();
        zmk_hid_mouse_buttons_release((zmk_mouse_button_flags_t)click);
        zmk_endpoint_send_mouse_report();
    }
}
static K_WORK_DEFINE(tp_work, tp_work_handler);

static void tp_tap_handler(struct k_work *work) {
    ARG_UNUSED(work);
    if (tp_first_tap) {
        tp_first_tap = false;
        atomic_set(&tp_click, MB1);
        k_work_submit(&tp_work);
    }
}
static K_WORK_DELAYABLE_DEFINE(tp_tap_work, tp_tap_handler);
#endif /* IS_ENABLED(CONFIG_ZMK_POINTING) */

static void touch_cb(struct input_event *evt, void *user_data) {
    ARG_UNUSED(user_data);

    switch (evt->code) {
    case INPUT_ABS_X:
        cur_x = evt->value;
        break;
    case INPUT_ABS_Y:
        cur_y = evt->value;
        break;
    case INPUT_BTN_TOUCH:
        if (evt->value && !active) {
            active = true;
            start_x = cur_x;
            start_y = cur_y;
            down_ms = k_uptime_get();
#if IS_ENABLED(CONFIG_ZMK_POINTING)
            tp_start_sx = panel_to_screen_x(start_x, start_y);
            tp_start_sy = panel_to_screen_y(start_x, start_y);
            tp_mode = TP_PENDING;
            tp_scroll_zone = (tp_rot & 1) ? (tp_start_sy >= TP_SCROLL_ZONE)
                                          : (tp_start_sx >= TP_SCROLL_ZONE);

            tp_drag_candidate = tp_first_tap;
            if (tp_drag_candidate) {
                k_work_cancel_delayable(&tp_tap_work);
            }
            tp_carry_x = 0;
            tp_carry_y = 0;
#endif
        } else if (!evt->value && active) {
            active = false;
#if IS_ENABLED(CONFIG_ZMK_POINTING)
            if (prospector_touchpad_active()) {
                /* A "tap" = finger lifted before committing to a drag (scroll or move). */
                if (tp_mode == TP_PENDING) {
                    if (prospector_touch_has_action(tp_start_sx, tp_start_sy)) {
                        tp_first_tap = false;
                        k_work_cancel_delayable(&tp_tap_work);
                        pending_sx = tp_start_sx;
                        pending_sy = tp_start_sy;
                        k_work_submit(&touch_work);
                    } else if (tp_first_tap) {
                        /* second tap within the window = right click */
                        tp_first_tap = false;
                        k_work_cancel_delayable(&tp_tap_work);
                        atomic_set(&tp_click, MB2);
                        k_work_submit(&tp_work);
                    } else {
                        /* first tap -> defer the left click, wait for a possible second */
                        tp_first_tap = true;
                        k_work_reschedule(&tp_tap_work, K_MSEC(TP_DTAP_MS));
                    }
                } else if (tp_mode == TP_DRAG) {
                    /* Drag-lock ends on lift: release the held button. */
                    atomic_set(&tp_drag_cmd, 2);
                    k_work_submit(&tp_work);
                }
                break; /* trackpad mode owns the release; skip cell dispatch */
            }
#endif
            if (iabs32(cur_x - start_x) < TOUCH_TAP_MAX_TRAVEL &&
                iabs32(cur_y - start_y) < TOUCH_TAP_MAX_TRAVEL) {
                pending_sx = panel_to_screen_x(cur_x, cur_y);
                pending_sy = panel_to_screen_y(cur_x, cur_y);
                pending_hold = (k_uptime_get() - down_ms) >= TOUCH_HOLD_MS;
                LOG_INF("tap raw(%d,%d) screen(%d,%d) -> cell %d%s",
                        cur_x, cur_y, pending_sx, pending_sy,
                        touch_cell(pending_sx, pending_sy), pending_hold ? " (hold)" : "");

                k_work_submit(&touch_work);
            }
        }
        break;
    default:
        break;
    }

#if IS_ENABLED(CONFIG_ZMK_POINTING)

    if (prospector_touchpad_active() && active &&
        (evt->code == INPUT_ABS_X || evt->code == INPUT_ABS_Y)) {
        int32_t sx = panel_to_screen_x(cur_x, cur_y);
        int32_t sy = panel_to_screen_y(cur_x, cur_y);
        if (tp_mode == TP_PENDING) {

            if (iabs32(sx - tp_start_sx) >= TP_MOVE_DEADZONE_PX ||
                iabs32(sy - tp_start_sy) >= TP_MOVE_DEADZONE_PX) {
                if (tp_scroll_zone) {
                    tp_mode = TP_SCROLL;
                    tp_scroll_ref = (tp_rot & 1) ? sx : sy;
                } else if (tp_drag_candidate) {
                    tp_first_tap = false; /* this touch is a drag now, not a tap */
                    k_work_cancel_delayable(&tp_tap_work);
                    tp_mode = TP_DRAG;
                    tp_prev_sx = sx; /* stream from here; the dead-zone px are dropped */
                    tp_prev_sy = sy;
                    atomic_set(&tp_drag_cmd, 1);
                    k_work_submit(&tp_work);
                } else {
                    tp_mode = TP_MOTION;
                    tp_prev_sx = sx; /* stream from here; the dead-zone px are dropped */
                    tp_prev_sy = sy;
                }
            }
        } else if (tp_mode == TP_MOTION || tp_mode == TP_DRAG) {
            int dx = sx - tp_prev_sx;
            int dy = sy - tp_prev_sy;
            tp_prev_sx = sx;
            tp_prev_sy = sy;
            if (dx || dy) {

                int32_t mult = (int32_t)tp_sens_level * TP_SENS_MULT256;
                tp_carry_x += dx * mult;
                tp_carry_y += dy * mult;
                int outx = tp_carry_x / 256;
                int outy = tp_carry_y / 256;
                tp_carry_x -= outx * 256;
                tp_carry_y -= outy * 256;
                if (outx || outy) {
                    atomic_add(&tp_dx, outx);
                    atomic_add(&tp_dy, outy);
                    k_work_submit(&tp_work);
                }
            }
        } else {
            int32_t s = (tp_rot & 1) ? sx : sy;
            int ds = s - tp_scroll_ref;
            if (ds >= TP_SCROLL_PX || ds <= -TP_SCROLL_PX) {
                int ticks = ds / TP_SCROLL_PX; /* signed */

                atomic_add(&tp_scroll, -ticks);
                tp_scroll_ref += ticks * TP_SCROLL_PX;
                k_work_submit(&tp_work);
            }
        }
    }
#endif
}
INPUT_CALLBACK_DEFINE(NULL, touch_cb, NULL);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(hynitron_cst816s) */
