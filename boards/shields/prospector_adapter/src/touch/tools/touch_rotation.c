/* 4-step display rotation (settings screen, 90deg CW per tap).
 *
 * Pure hardware scan-out change (MADCTL via display_set_orientation) + an LVGL
 * logical-resolution swap -- no software rotation, no extra buffers. The glass is
 * centred in the controller RAM (y-offset 20 of 40 spare) and the st7789v driver
 * re-derives the margins per orientation, so all four orientations line up. If the
 * two portraits come out swapped vs the touch on hardware, swap the middle entries
 * of rot_to_panel[] AND cases 1/3 in touch_input.c's transform.
 *
 * status_screen_reflow() (../status_screen.c) repositions the NORMAL screen's
 * widgets per orientation; the touch UI (build_view) is portrait-aware too. */

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include "../touch_ui.h"

uint8_t ui_rot; /* 0..3 = 0/90/180/270 deg CW; 0 = calibrated landscape */

static const enum display_orientation rot_to_panel[4] = {
    DISPLAY_ORIENTATION_ROTATED_270, /* calibrated landscape */
    DISPLAY_ORIENTATION_ROTATED_180, /* portrait A */
    DISPLAY_ORIENTATION_ROTATED_90,  /* landscape flipped 180 */
    DISPLAY_ORIENTATION_NORMAL,      /* portrait B */
};

/* Told the UI orientation so touch coords rotate with the image. Weak no-op keeps
 * the layout linkable without the touch driver. */
__weak void prospector_touch_set_orientation(int rot) { ARG_UNUSED(rot); }

void settings_apply_rotation(void) {
    const struct device *disp = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(disp)) {
        return;
    }
    display_set_orientation(disp, rot_to_panel[ui_rot]);
    lv_display_set_resolution(lv_display_get_default(), scr_w(), scr_h());
    lv_obj_set_size(touch_overlay, scr_w(), scr_h());
    prospector_touch_set_orientation(ui_rot);
    status_screen_reflow(); /* re-position the NORMAL screen's widgets */
    build_view(cur_view);   /* re-lay the current touch screen for the new dims */
    lv_obj_invalidate(lv_screen_active());
}
