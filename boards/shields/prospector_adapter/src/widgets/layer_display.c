#include "layer_display.h"

#include <zmk/display.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/keymap.h>

#include "display_colors.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct layer_display_state {
    uint8_t index;
};

static void layer_display_update_cb(struct layer_display_state state) {
    struct zmk_widget_layer_display *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        for (int i = 0; i < LAYER_DOT_COUNT; i++) {
            lv_color_t color = (i == state.index)
                ? lv_color_hex(COLOR_LAYER_DOT_ACTIVE)
                : lv_color_hex(COLOR_LAYER_DOT_INACTIVE);
            lv_obj_set_style_bg_color(widget->dots[i], color, LV_PART_MAIN);
        }
    }
}

static struct layer_display_state layer_display_get_state(const zmk_event_t *eh) {
    return (struct layer_display_state){
        .index = zmk_keymap_highest_layer_active(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_display, struct layer_display_state,
                            layer_display_update_cb, layer_display_get_state)
ZMK_SUBSCRIPTION(widget_layer_display, zmk_layer_state_changed);

/* Resize the dot row (portrait rotation): dots shrink to fit, keeping the gap. */
void zmk_widget_layer_display_set_width(struct zmk_widget_layer_display *widget, int width) {
    const int dot_gap = 3;
    int dot_width = (width - (LAYER_DOT_COUNT - 1) * dot_gap) / LAYER_DOT_COUNT;
    lv_obj_set_width(widget->obj, width);
    for (int i = 0; i < LAYER_DOT_COUNT; i++) {
        lv_obj_set_size(widget->dots[i], dot_width, 6);
        lv_obj_set_pos(widget->dots[i], i * (dot_width + dot_gap), 0);
    }
}

int zmk_widget_layer_display_init(struct zmk_widget_layer_display *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 260, 6);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);

    for (int i = 0; i < LAYER_DOT_COUNT; i++) {
        widget->dots[i] = lv_obj_create(widget->obj);
        lv_obj_set_style_bg_color(widget->dots[i], lv_color_hex(COLOR_LAYER_DOT_INACTIVE), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(widget->dots[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(widget->dots[i], 0, LV_PART_MAIN);
        lv_obj_set_style_radius(widget->dots[i], 2, LV_PART_MAIN);
        lv_obj_set_style_pad_all(widget->dots[i], 0, LV_PART_MAIN);
    }
    zmk_widget_layer_display_set_width(widget, 260);

    sys_slist_append(&widgets, &widget->node);
    widget_layer_display_init();

    return 0;
}

lv_obj_t *zmk_widget_layer_display_obj(struct zmk_widget_layer_display *widget) {
    return widget->obj;
}
