#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_modifier_indicator {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *mod_labels[4];
};

int zmk_widget_modifier_indicator_init(struct zmk_widget_modifier_indicator *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_modifier_indicator_obj(struct zmk_widget_modifier_indicator *widget);
/* Resize (portrait rotation): the row uses LV_FLEX_ALIGN_SPACE_BETWEEN, so
 * changing the container width alone re-spaces the gaps between mod keys --
 * no child repositioning needed (unlike wpm_meter/layer_display's absolute-
 * positioned children). */
void zmk_widget_modifier_indicator_set_width(struct zmk_widget_modifier_indicator *widget, int width);
