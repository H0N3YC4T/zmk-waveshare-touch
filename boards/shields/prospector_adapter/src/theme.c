/* Runtime UI theme: role -> colour lookup, category base swapping,
 * settings persistence ("prospector_theme/bases"). */

#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

#include "theme.h"

__weak void theme_changed(void) {}

/* Default palette */
static const uint32_t theme_classic[THEME_ROLE_COUNT] = {
    [THEME_PRIMARY] = 0xc8a2c8,
    [THEME_PRIMARY_DIM] = 0x242424,
    [THEME_SECONDARY] = 0xa8d0e6,
    [THEME_SECONDARY_BRIGHT] = 0xb1e5f0,
    [THEME_SECONDARY_DIM] = 0x3b527c,
    [THEME_FOCUS] = 0xf5e08c,
    [THEME_FOCUS_BRIGHT] = 0xffbf00,
    [THEME_ACCEPT] = 0xa8e6b8,
    [THEME_DENY] = 0xc2526a,
    [THEME_INCREMENT] = 0xa8e6b8,
    [THEME_DECREMENT] = 0xf5e08c,
    [THEME_BACKGROUND] = 0x000000,
    [THEME_SURFACE] = 0x101216,
    [THEME_SURFACE_LOW] = 0x0b0d10,
    [THEME_OUTLINE] = 0x2e3238,
    [THEME_MUTED] = 0x505050,
    [THEME_MUTED_DIM] = 0x303030,
};

/* every role follows exactly one category base */
static const enum theme_category role_cat[THEME_ROLE_COUNT] = {
    [THEME_PRIMARY] = THEME_CAT_PRIMARY,
    [THEME_PRIMARY_DIM] = THEME_CAT_PRIMARY,
    [THEME_SECONDARY] = THEME_CAT_SECONDARY,
    [THEME_SECONDARY_BRIGHT] = THEME_CAT_SECONDARY,
    [THEME_SECONDARY_DIM] = THEME_CAT_SECONDARY,
    [THEME_FOCUS] = THEME_CAT_FOCUS,
    [THEME_FOCUS_BRIGHT] = THEME_CAT_FOCUS,
    [THEME_ACCEPT] = THEME_CAT_ACCEPT,
    [THEME_DENY] = THEME_CAT_DENY,
    [THEME_INCREMENT] = THEME_CAT_INCREMENT,
    [THEME_DECREMENT] = THEME_CAT_DECREMENT,
    [THEME_BACKGROUND] = THEME_CAT_BACKGROUND,
    [THEME_SURFACE] = THEME_CAT_BACKGROUND,
    [THEME_SURFACE_LOW] = THEME_CAT_BACKGROUND,
    [THEME_OUTLINE] = THEME_CAT_BACKGROUND,
    [THEME_MUTED] = THEME_CAT_BACKGROUND,
    [THEME_MUTED_DIM] = THEME_CAT_BACKGROUND,
};

/* vivid preset: each classic base's neon swatch counterpart */
static const uint32_t theme_vivid[THEME_CAT_COUNT] = {
    [THEME_CAT_PRIMARY] = 0xc026ff,
    [THEME_CAT_SECONDARY] = 0x2f6bff,
    [THEME_CAT_FOCUS] = 0xffd500,
    [THEME_CAT_ACCEPT] = 0x00e050,
    [THEME_CAT_DENY] = 0xff3b30,
    [THEME_CAT_INCREMENT] = 0x00e050,
    [THEME_CAT_DECREMENT] = 0xffd500,
    [THEME_CAT_BACKGROUND] = 0x000000,
};

/* the role holding each category's base colour */
static const enum theme_role cat_base[THEME_CAT_COUNT] = {
    [THEME_CAT_PRIMARY] = THEME_PRIMARY,
    [THEME_CAT_SECONDARY] = THEME_SECONDARY,
    [THEME_CAT_FOCUS] = THEME_FOCUS,
    [THEME_CAT_ACCEPT] = THEME_ACCEPT,
    [THEME_CAT_DENY] = THEME_DENY,
    [THEME_CAT_INCREMENT] = THEME_INCREMENT,
    [THEME_CAT_DECREMENT] = THEME_DECREMENT,
    [THEME_CAT_BACKGROUND] = THEME_BACKGROUND,
};

static uint32_t palette[THEME_ROLE_COUNT] = {
    [THEME_PRIMARY] = 0xc8a2c8,
    [THEME_PRIMARY_DIM] = 0x242424,
    [THEME_SECONDARY] = 0xa8d0e6,
    [THEME_SECONDARY_BRIGHT] = 0xb1e5f0,
    [THEME_SECONDARY_DIM] = 0x3b527c,
    [THEME_FOCUS] = 0xf5e08c,
    [THEME_FOCUS_BRIGHT] = 0xffbf00,
    [THEME_ACCEPT] = 0xa8e6b8,
    [THEME_DENY] = 0xc2526a,
    [THEME_INCREMENT] = 0xa8e6b8,
    [THEME_DECREMENT] = 0xf5e08c,
    [THEME_BACKGROUND] = 0x000000,
    [THEME_SURFACE] = 0x101216,
    [THEME_SURFACE_LOW] = 0x0b0d10,
    [THEME_OUTLINE] = 0x2e3238,
    [THEME_MUTED] = 0x505050,
    [THEME_MUTED_DIM] = 0x303030,
};

uint32_t theme_color(enum theme_role role)
{
  return (role < THEME_ROLE_COUNT) ? palette[role] : 0xffffff;
}

uint32_t theme_get_base(enum theme_category cat)
{
  return (cat < THEME_CAT_COUNT) ? palette[cat_base[cat]] : 0xffffff;
}

static uint8_t shift_channel(uint8_t base, uint8_t ref_base, uint8_t ref_shade)
{
  int v = (int)base + ((int)ref_shade - (int)ref_base);
  return (uint8_t)(v < 0 ? 0 : v > 255 ? 255
                                       : v);
}

/* shade = new base + (classic shade - classic base), per RGB channel */
static uint32_t shift_shade(uint32_t base, uint32_t ref_base, uint32_t ref_shade)
{
  uint32_t r = shift_channel(base >> 16, ref_base >> 16, ref_shade >> 16);
  uint32_t g = shift_channel(base >> 8, ref_base >> 8, ref_shade >> 8);
  uint32_t b = shift_channel(base, ref_base, ref_shade);
  return (r << 16) | (g << 8) | b;
}

static void theme_apply_base(enum theme_category cat, uint32_t base_rgb)
{
  uint32_t ref_base = theme_classic[cat_base[cat]];
  for (int r = 0; r < THEME_ROLE_COUNT; r++)
  {
    if (role_cat[r] != cat)
    {
      continue;
    }
    palette[r] = (r == cat_base[cat]) ? base_rgb
                                      : shift_shade(base_rgb, ref_base, theme_classic[r]);
  }
}

#if IS_ENABLED(CONFIG_SETTINGS)

static void theme_save_cb(struct k_work *work)
{
  uint32_t bases[THEME_CAT_COUNT];
  for (int c = 0; c < THEME_CAT_COUNT; c++)
  {
    bases[c] = palette[cat_base[c]];
  }
  settings_save_one("prospector_theme/bases", bases, sizeof(bases));
}
static K_WORK_DELAYABLE_DEFINE(theme_save_work, theme_save_cb);

static int theme_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                              void *cb_arg)
{
  uint32_t bases[THEME_CAT_COUNT];
  if (settings_name_steq(name, "bases", NULL) && len == sizeof(bases))
  {
    if (read_cb(cb_arg, bases, sizeof(bases)) == sizeof(bases))
    {
      for (int c = 0; c < THEME_CAT_COUNT; c++)
      {
        theme_apply_base(c, bases[c]);
      }
    }
  }
  return 0;
}

static int theme_settings_commit(void)
{
  theme_changed(); /* no-op at boot (screen not built yet); palette is ready */
  return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(prospector_theme, "prospector_theme", NULL, theme_settings_set,
                               theme_settings_commit, NULL);

#endif /* IS_ENABLED(CONFIG_SETTINGS) */

void theme_reset_classic(void)
{
  for (int c = 0; c < THEME_CAT_COUNT; c++)
  {
    theme_apply_base(c, theme_classic[cat_base[c]]);
  }
  theme_changed();
#if IS_ENABLED(CONFIG_SETTINGS)
  k_work_schedule(&theme_save_work, K_SECONDS(2));
#endif
}

void theme_reset_vivid(void)
{
  for (int c = 0; c < THEME_CAT_COUNT; c++)
  {
    theme_apply_base(c, theme_vivid[c]);
  }
  theme_changed();
#if IS_ENABLED(CONFIG_SETTINGS)
  k_work_schedule(&theme_save_work, K_SECONDS(2));
#endif
}

bool theme_is_classic(void)
{
  for (int c = 0; c < THEME_CAT_COUNT; c++)
  {
    if (palette[cat_base[c]] != theme_classic[cat_base[c]])
    {
      return false;
    }
  }
  return true;
}

void theme_set_base(enum theme_category cat, uint32_t base_rgb)
{
  if (cat >= THEME_CAT_COUNT)
  {
    return;
  }
  theme_apply_base(cat, base_rgb);
  theme_changed();
#if IS_ENABLED(CONFIG_SETTINGS)
  k_work_schedule(&theme_save_work, K_SECONDS(2)); /* debounce flash writes */
#endif
}
