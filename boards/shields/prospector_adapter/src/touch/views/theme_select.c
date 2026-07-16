/* ----------------------------- THEME SELECT -------------------------------- */
/* two-page category list -> one shared colour swatch page (category passed in) */

#include "../touch_ui.h"

static const struct view_def view_swatch;

/* 0-6 pastel red->green->blue (classic defaults kept) + white; 8-14 neon + black */
static const uint32_t swatch_colors[16] = {
    0xc2526a, /* rose (deny)        */
    0xf5b98c, /* apricot            */
    0xf5e08c, /* chardonnay (focus) */
    0xa8e6b8, /* mint (accept)      */
    0x8cd9c8, /* teal               */
    0xa8d0e6, /* sky (secondary)    */
    0xc8a2c8, /* lilac (primary)    */
    0xf2f2f0, /* white              */
    0xff3b30, /* neon red           */
    0xff8c00, /* neon orange        */
    0xffd500, /* neon yellow        */
    0x00e050, /* neon green         */
    0x00d8d8, /* neon cyan          */
    0x2f6bff, /* neon blue          */
    0xc026ff, /* neon violet        */
    0x000000, /* black              */
};

static enum theme_category swatch_cat;
static int categories_page; /* theme page to return to after picking */

/* ------------------------------ categories -------------------------------- */
static void open_swatch(int cat)
{
  swatch_cat = (enum theme_category)cat;
  categories_page = cur_page;
  show_view(&view_swatch);
}

/* one-tap preset toggle: classic pastels <-> their neon counterparts */
static void tap_preset(int cell)
{
  ARG_UNUSED(cell);
  if (theme_is_classic())
  {
    theme_reset_vivid();
  }
  else
  {
    theme_reset_classic();
  }
  build_view(cur_view);
}

static const struct page_cell theme_p0[] = {
    {0, 0, 1, 1, NULL, &icon_up, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_settings},
    {0, 1, 1, 2, "PRIMARY", NULL, THEME_PRIMARY, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_PRIMARY}},
    {1, 1, 1, 2, "SECONDARY", NULL, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_SECONDARY}},
    {2, 0, 1, 1, NULL, &icon_down, THEME_FOCUS, ACT_NEXT_PAGE},
    {2, 1, 1, 2, "FOCUS", NULL, THEME_FOCUS, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_FOCUS}},
    {0}};

static const struct page_cell theme_p1[] = {
    {0, 0, 1, 1, NULL, &icon_up, THEME_FOCUS, ACT_PREV_PAGE},
    {0, 1, 1, 2, "INCREMENT", NULL, THEME_INCREMENT, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_INCREMENT}},
    {1, 1, 1, 2, "DECREMENT", NULL, THEME_DECREMENT, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_DECREMENT}},
    {2, 0, 1, 1, NULL, &icon_down, THEME_FOCUS, ACT_NEXT_PAGE},
    {2, 1, 1, 2, "BACKGROUND", NULL, THEME_MUTED, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_BACKGROUND}},
    {0}};

static const struct page_cell theme_p2[] = {
    {0, 0, 1, 1, NULL, &icon_up, THEME_FOCUS, ACT_PREV_PAGE},
    {0, 1, 1, 2, "ACCEPT", NULL, THEME_ACCEPT, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_ACCEPT}},
    {1, 1, 1, 2, "DENY", NULL, THEME_DENY, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_DENY}},
    {2, 0, 1, 1, NULL, &icon_down, THEME_DENY, ACT_GO_VIEW, .arg.view = &view_settings},
    {2, 1, 1, 2, "PASTEL", NULL, THEME_MUTED, ACT_CUSTOM, .arg.func = tap_preset},
    {0}};

static const struct page_cell *const theme_pages[] = {theme_p1, theme_p2};

/* category buttons read one size smaller than the default cell face */
static void build_theme(void)
{
  for (int i = 0; i < 8; i++)
  {
    lv_obj_t *b = cur_view_btns[i];
    if (b == NULL)
    {
      continue;
    }
    lv_obj_t *l = lv_obj_get_child(b, 0);
    if (l != NULL && lv_obj_check_type(l, &lv_label_class))
    {
      lv_obj_set_style_text_font(l, &lv_font_montserrat_16, LV_PART_MAIN);
    }
  }
  /* preset toggle names its destination */
  if (cur_page == 2 && cur_view_btns[4] != NULL)
  {
    lv_obj_t *l = lv_obj_get_child(cur_view_btns[4], 0);
    if (l != NULL && lv_obj_check_type(l, &lv_label_class))
    {
      lv_label_set_text(l, theme_is_classic() ? "VIVID" : "PASTEL");
    }
  }
}

const struct view_def view_theme = {
    .cells = theme_p0,
    .build = build_theme,
    .pages = theme_pages,
    .num_pages = 3,
    .keeps_mods = true,
    .idle_timeout = true,
};

/* ------------------------------- swatches --------------------------------- */

static void back_to_categories(void)
{
  int page = categories_page;
  show_view(&view_theme);
  if (page > 0)
  {
    cur_page = page;
    build_view(&view_theme);
  }
}

/* backgrounds store a 95% black / 5% colour tint, not the raw pastel */
static uint32_t swatch_value(uint32_t c)
{
  if (swatch_cat != THEME_CAT_BACKGROUND)
  {
    return c;
  }
  return ((c >> 16 & 0xff) * 13 / 256) << 16 | ((c >> 8 & 0xff) * 13 / 256) << 8 |
         (c & 0xff) * 13 / 256;
}

static void pick_color(int idx)
{
  uint32_t color = swatch_value(swatch_colors[idx]);
  if (theme_get_base(swatch_cat) != color)
  {
    theme_set_base(swatch_cat, color);
  }
  back_to_categories();
}

/* fill = colour muted into the background, white dot on the current selection */
static void build_swatch(void)
{
  uint32_t bg = theme_color(THEME_BACKGROUND);
  uint32_t current = theme_get_base(swatch_cat);
  for (int i = 0; i < 16; i++)
  {
    lv_obj_t *b = cur_view_btns[i];
    if (b == NULL)
    {
      continue;
    }
    uint32_t c = swatch_colors[i]; /* face keeps the full colour */
    uint32_t rim = ((c >> 16 & 0xff) + (c >> 8 & 0xff) + (c & 0xff)) < 96
                       ? theme_color(THEME_MUTED)
                       : c;
    lv_obj_set_style_border_color(b, lv_color_hex(rim), LV_PART_MAIN);
    lv_obj_set_style_bg_color(b, lv_color_mix(lv_color_hex(c), lv_color_hex(bg), 128),
                              LV_PART_MAIN);
    if (swatch_value(c) == current)
    {
      lv_obj_t *dot = lv_obj_create(b);
      if (dot != NULL)
      {
        lv_obj_set_size(dot, 12, 12);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_color(dot, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        lv_obj_center(dot);
      }
    }
  }
}

#define SWATCH(r, c, i)                                                             \
  {                                                                                 \
    r, c, 1, 1, " ", NULL, THEME_MUTED, ACT_CUSTOM_VAL, .arg.custom = { pick_color, \
                                                                        i }         \
  }

static const struct page_cell swatch_cells[] = {
    SWATCH(0, 0, 0), SWATCH(0, 1, 1), SWATCH(0, 2, 2), SWATCH(0, 3, 3), 
    SWATCH(1, 0, 4), SWATCH(1, 1, 5), SWATCH(1, 2, 6), SWATCH(1, 3, 7), 
    SWATCH(2, 0, 8), SWATCH(2, 1, 9), SWATCH(2, 2, 10), SWATCH(2, 3, 11), 
    SWATCH(3, 0, 12), SWATCH(3, 1, 13), SWATCH(3, 2, 14), SWATCH(3, 3, 15), 
    {0}};

static const struct view_def view_swatch = {
    .cells = swatch_cells,
    .build = build_swatch,
    .idle_timeout = true,
};
