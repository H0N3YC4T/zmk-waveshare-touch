/* ----------------------------- THEME SELECT -------------------------------- */
/* two-page category list -> one shared colour swatch page (category passed in) */

#include "../touch_ui.h"

static const struct view_def view_swatch;

/* preset bases: the six classic defaults + cream / apricot / magenta */
static const uint32_t swatch_colors[9] = {
    0xc8a2c8, /* lilac    */
    0xa8d0e6, /* sky      */
    0xf5e08c, /* pastel yellow */
    0xa8e6b8, /* mint     */
    0xc2526a, /* rose     */
    0xe08cd0, /* magenta  */
    0xf5b98c, /* apricot  */
    0xf0e8d8, /* cream    */
    0x000000, /* black    */
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

static const struct page_cell theme_p0[] = {
    {0, 0, 1, 1, NULL, &icon_up,   THEME_DENY,      ACT_GO_VIEW, .arg.view = &view_settings},
    {0, 1, 1, 2, "PRIMARY",  NULL, THEME_PRIMARY,   ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_PRIMARY}},
    {1, 1, 1, 2, "SECONDARY", NULL, THEME_SECONDARY, ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_SECONDARY}},
    {2, 0, 1, 1, NULL, &icon_down, THEME_FOCUS,     ACT_NEXT_PAGE},
    {2, 1, 1, 2, "FOCUS",    NULL, THEME_FOCUS,     ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_FOCUS}},
    {0}
};

static const struct page_cell theme_p1[] = {
    {0, 0, 1, 1, NULL, &icon_up,   THEME_FOCUS,     ACT_PREV_PAGE},
    /* background's own base can be invisible on itself -- outline muted */
    {0, 1, 1, 2, "BACKGROUND", NULL, THEME_MUTED,   ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_BACKGROUND}},
    {1, 1, 1, 2, "ACCEPT",   NULL, THEME_ACCEPT,    ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_ACCEPT}},
    {2, 0, 1, 1, NULL, &icon_down, THEME_DENY,      ACT_GO_VIEW, .arg.view = &view_settings},
    {2, 1, 1, 2, "DENY",     NULL, THEME_DENY,      ACT_CUSTOM_VAL, .arg.custom = {open_swatch, THEME_CAT_DENY}},
    {0}
};

static const struct page_cell *const theme_pages[] = {theme_p1};

const struct view_def view_theme = {
    .cells = theme_p0,
    .pages = theme_pages,
    .num_pages = 2,
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

static void pick_color(int idx)
{
  uint32_t color = swatch_colors[idx];
  if (theme_get_base(swatch_cat) != color)
  {
    theme_set_base(swatch_cat, color); /* applies live + persists */
  }
  back_to_categories(); /* picking the current colour just backs out */
}

/* swatch face: border in the colour, fill = colour muted into the background,
 * white dot on the current selection */
static void build_swatch(void)
{
  uint32_t bg = theme_color(THEME_BACKGROUND);
  uint32_t current = theme_get_base(swatch_cat);
  for (int i = 0; i < 9; i++)
  {
    lv_obj_t *b = cur_view_btns[i];
    if (b == NULL)
    {
      continue;
    }
    uint32_t c = swatch_colors[i];
    /* very dark swatches vanish against the background -- keep a visible rim */
    uint32_t rim = ((c >> 16 & 0xff) + (c >> 8 & 0xff) + (c & 0xff)) < 96
                       ? theme_color(THEME_MUTED)
                       : c;
    lv_obj_set_style_border_color(b, lv_color_hex(rim), LV_PART_MAIN);
    lv_obj_set_style_bg_color(b, lv_color_mix(lv_color_hex(c), lv_color_hex(bg), 128),
                              LV_PART_MAIN);
    if (c == current)
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

#define SWATCH(r, c, i) \
    {r, c, 1, 1, " ", NULL, THEME_MUTED, ACT_CUSTOM_VAL, .arg.custom = {pick_color, i}}

static const struct page_cell swatch_cells[] = {
    SWATCH(0, 0, 0), SWATCH(0, 1, 1), SWATCH(0, 2, 2),
    SWATCH(1, 0, 3), SWATCH(1, 1, 4), SWATCH(1, 2, 5),
    SWATCH(2, 0, 6), SWATCH(2, 1, 7), SWATCH(2, 2, 8),
    {0}
};

static const struct view_def view_swatch = {
    .cells = swatch_cells,
    .build = build_swatch,
    .keeps_mods = true,
    .idle_timeout = true,
};
