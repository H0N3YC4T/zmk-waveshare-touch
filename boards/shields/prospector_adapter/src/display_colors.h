#pragma once

// ── Base ─────────────────────────────────────────────────────────────────────
#define COLOR_BACKGROUND 0x000000 // screen / label background

// ── Primary UI accent colours ─────────────────────────────────────────────────
// Used throughout the touch views; widgets use named role colours below.
#define COLOR_PRIMARY   0xc8a2c8 // lilac    – default key / active element
#define COLOR_ACCENT    0xa8d0e6 // sky blue – navigation, rotation, armed state
#define COLOR_RED       0xc2526a // rose     – back/exit, battery LOW, alert
#define COLOR_GREEN     0xa8e6b8 // mint     – settings +, affirmative
#define COLOR_YELLOW    0xf5e08c // pastel   – settings -, battery MID

// ── Touch UI surface colours ──────────────────────────────────────────────────
#define COLOR_CHARCOAL   0x101216 // button fill
#define COLOR_DARK_GREY  0x303030 // dim legend / hint text
#define COLOR_GREY       0x505050 // greyed-out controls, brighter hints
#define COLOR_NEAR_BLACK 0x0b0d10 // scroll-track fill (below button fill)
#define COLOR_SLATE      0x2e3238 // scroll-track outline

// ── Modifier indicator ────────────────────────────────────────────────────────
#define COLOR_MOD_ACTIVE     0xb1e5f0
#define COLOR_MOD_INACTIVE   0x3b527c
#define COLOR_MOD_SEPARATOR  0x606060
#define COLOR_MOD_CAPS_WORD  0xffbf00

// ── WPM meter ─────────────────────────────────────────────────────────────────
#define COLOR_WPM_BAR_ACTIVE    COLOR_PRIMARY // bar active = primary accent
#define COLOR_WPM_BAR_INACTIVE  0x242424

// ── Layer display ─────────────────────────────────────────────────────────────
#define COLOR_LAYER_TEXT          0xffffff
#define COLOR_LAYER_DOT_ACTIVE    0xe0e0e0
#define COLOR_LAYER_DOT_INACTIVE  0x575757

// ── Battery tiers: HIGH >60% (accent), MID 30-60% (yellow), LOW <30% (red) ──
#define COLOR_BATTERY_RING                0x32424d
#define COLOR_BATTERY_BG                  0x505050
#define COLOR_BATTERY_LABEL               0xffffff
#define COLOR_BATTERY_DISCONNECTED_FILL   0x383c42
#define COLOR_BATTERY_DISCONNECTED_RING   0x282c30
#define COLOR_BATTERY_DISCONNECTED_LABEL  0x000000
#define COLOR_BATTERY_MID_FILL            0xf5e08c // same value as COLOR_YELLOW
#define COLOR_BATTERY_MID_RING            0x4a4530
#define COLOR_BATTERY_LOW_RING            0x4a2730

// ── Output widget ─────────────────────────────────────────────────────────────
#define COLOR_USB_ACTIVE_BG         0xb9b9a7
#define COLOR_USB_INACTIVE_BG       0x4F4F40
#define COLOR_BLE_ACTIVE_BG         0x569FA7
#define COLOR_BLE_INACTIVE_BG       0x353f40
#define COLOR_OUTPUT_ACTIVE_TEXT    0x000000
#define COLOR_OUTPUT_INACTIVE_TEXT  0x7b7d93
#define COLOR_SLOT_ACTIVE_BG        0x7b7d93
#define COLOR_SLOT_INACTIVE_BG      0x353640
#define COLOR_SLOT_TEXT             0xffffff
