
#include <lvgl.h>
#include "theme.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "modifier_indicator.h"
#include "wpm_meter.h"
#include "layer_display.h"
#include "battery_circles.h"
#include "output.h"
#include "touch_ui.h"

LOG_MODULE_REGISTER(prospector_touch_ui, LOG_LEVEL_INF);

static struct zmk_widget_modifier_indicator modifier_indicator_widget;
static struct zmk_widget_wpm_meter wpm_meter_widget;
static struct zmk_widget_layer_display layer_display_widget;
static struct zmk_widget_battery_circles battery_circles_widget;
static struct zmk_widget_output output_widget;

void status_screen_reflow(void) {
    if (ui_rot & 1) {
        zmk_widget_modifier_indicator_set_width(&modifier_indicator_widget, 200);
        lv_obj_set_pos(zmk_widget_modifier_indicator_obj(&modifier_indicator_widget), 20, 8);
        zmk_widget_wpm_meter_set_width(&wpm_meter_widget, 220);
        lv_obj_set_pos(zmk_widget_wpm_meter_obj(&wpm_meter_widget), 10, 40);
        zmk_widget_layer_display_set_width(&layer_display_widget, 220);
        lv_obj_set_pos(zmk_widget_layer_display_obj(&layer_display_widget), 10, 140);
        zmk_widget_battery_circles_set_stacked(true);
        zmk_widget_output_set_stacked(true);
        lv_obj_set_pos(zmk_widget_battery_circles_obj(&battery_circles_widget), 10, 152);
        lv_obj_set_pos(zmk_widget_output_obj(&output_widget), 84, 152);
    } else {
        zmk_widget_modifier_indicator_set_width(&modifier_indicator_widget, 230);
        lv_obj_set_pos(zmk_widget_modifier_indicator_obj(&modifier_indicator_widget), 25, 8);
        zmk_widget_wpm_meter_set_width(&wpm_meter_widget, 260);
        lv_obj_set_pos(zmk_widget_wpm_meter_obj(&wpm_meter_widget), 10, 42);
        zmk_widget_layer_display_set_width(&layer_display_widget, 260);
        lv_obj_set_pos(zmk_widget_layer_display_obj(&layer_display_widget), 10, 142);
        zmk_widget_battery_circles_set_stacked(false);
        zmk_widget_output_set_stacked(false);
        lv_obj_set_pos(zmk_widget_battery_circles_obj(&battery_circles_widget), 11, 170);
        lv_obj_set_pos(zmk_widget_output_obj(&output_widget), 148, 170);
    }
}

static lv_obj_t *status_screen_obj;

/* live retheme: restyle the status widgets in place (touch views rebuild
 * themselves via build_view) */
void theme_changed(void) {
    if (status_screen_obj == NULL) {
        return;
    }
    lv_obj_set_style_bg_color(status_screen_obj, lv_color_hex(theme_color(THEME_BACKGROUND)),
                              LV_PART_MAIN);
    zmk_widget_wpm_meter_retheme();
    zmk_widget_modifier_indicator_retheme();
    zmk_widget_battery_circles_retheme();
    zmk_widget_output_retheme();
}

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);
    status_screen_obj = screen;
    lv_obj_set_style_bg_color(screen, lv_color_hex(theme_color(THEME_BACKGROUND)), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, 255, LV_PART_MAIN);

    zmk_widget_modifier_indicator_init(&modifier_indicator_widget, screen);
    zmk_widget_wpm_meter_init(&wpm_meter_widget, screen);
    zmk_widget_layer_display_init(&layer_display_widget, screen);
    zmk_widget_battery_circles_init(&battery_circles_widget, screen);
    zmk_widget_output_init(&output_widget, screen);
    status_screen_reflow();

    touch_ui_attach(screen);
    touch_prefs_apply(); /* restore saved brightness/sensitivity/rotation */

    return screen;
}
