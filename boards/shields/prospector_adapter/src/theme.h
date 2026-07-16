#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Semantic UI colour roles. Views and widgets name the ROLE they want; the
 * active palette resolves it at draw time, so a future theme settings page can
 * swap category bases at runtime. Inherent colours (battery tiers, BT blue,
 * output/layer widgets) live in display_colors.h and do NOT follow the theme. */

enum theme_role
{
  THEME_PRIMARY,          /* key/active elements (classic: lilac) */
  THEME_PRIMARY_DIM,      /* wpm meter track */
  THEME_SECONDARY,        /* navigation, armed state (classic: sky blue) */
  THEME_SECONDARY_BRIGHT, /* modifier indicator active */
  THEME_SECONDARY_DIM,    /* modifier indicator inactive */
  THEME_FOCUS,            /* attention accents (classic: pastel yellow) */
  THEME_FOCUS_BRIGHT,     /* caps-word indicator */
  THEME_ACCEPT,           /* affirmative controls (classic: mint) */
  THEME_DENY,             /* back/exit/destructive (classic: rose) */
  THEME_INCREMENT,        /* increment controls */
  THEME_DECREMENT,        /* decrement controls */
  THEME_BACKGROUND,       /* screen background */
  THEME_SURFACE,          /* button fill */
  THEME_SURFACE_LOW,      /* recessed fill (scroll track) */
  THEME_OUTLINE,          /* scroll-track outline */
  THEME_MUTED,            /* greyed-out controls, brighter hints */
  THEME_MUTED_DIM,        /* dim legend / hint text */
  THEME_ROLE_COUNT
};

/* One selectable slot per future settings-page category; every role above
 * belongs to a category and its shades follow that category's base. */
enum theme_category
{
  THEME_CAT_PRIMARY,
  THEME_CAT_SECONDARY,
  THEME_CAT_FOCUS,
  THEME_CAT_ACCEPT,
  THEME_CAT_DENY,
  THEME_CAT_INCREMENT,
  THEME_CAT_DECREMENT,
  THEME_CAT_BACKGROUND,
  THEME_CAT_COUNT
};

uint32_t theme_color(enum theme_role role);

/* Set a category's base colour; linked shades keep their per-channel offset
 * from the classic palette (dark stays "that much darker" than its base).
 * Applies live (no restart), notifies theme_changed(), persists to settings. */
void theme_set_base(enum theme_category cat, uint32_t base_rgb);
uint32_t theme_get_base(enum theme_category cat);

/* All categories back to the classic palette (applies live + persists). */
void theme_reset_classic(void);

/* All categories to the vivid preset: each classic base's neon swatch
 * counterpart, background stays black (applies live + persists). */
void theme_reset_vivid(void);

/* true while every category base still matches the classic palette */
bool theme_is_classic(void);

/* Notified after any palette change (live set_base or boot settings load).
 * Weak no-op in theme.c; the status screen overrides it to restyle live
 * widgets. Touch views restyle themselves via build_view() as usual. */
void theme_changed(void);
