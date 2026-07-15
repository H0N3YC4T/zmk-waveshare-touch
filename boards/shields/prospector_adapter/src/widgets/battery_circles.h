#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

extern const lv_image_dsc_t icon_lightning_24;

struct zmk_widget_battery_circles
{
  sys_snode_t node;
  lv_obj_t *obj;
  bool initialized;
};

int zmk_widget_battery_circles_init(struct zmk_widget_battery_circles *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_battery_circles_obj(struct zmk_widget_battery_circles *widget);
void zmk_widget_battery_circles_retheme(void); /* re-apply themed colours in place */
void zmk_widget_battery_circles_place(int w, int h, int x0, int y0, int x1, int y1);
