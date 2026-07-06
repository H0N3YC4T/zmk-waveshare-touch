# Fork changes — H0N3YC4T/prospector-zmk-module @ feat/new-status-screens

Everything this fork adds on top of carrefinho/prospector-zmk-module, in one place. This file
carries the detailed rationale and history that the code comments deliberately don't, so future
edits have full context. Companion docs live in the keyboard repo
(Keyboard-Prototype_Mk1: docs/information.md + docs/issues.md).

Consumed by the keyboard repo via an exact-SHA pin in `config/west.yml` — every change here
needs a commit + push + pin bump there. Hardware: Seeed XIAO nRF52840 + Waveshare 1.69" LCD
(280x240, ST7789V, glass corners R5.15mm ≈ 44px) + CST816S touch, OPERATOR layout.

## Files changed vs upstream

### `src/layouts/operator/status_screen.c` — the entire touch UI (biggest change)

**Views (menu tree, after the 2026-07-06 swap):** NORMAL → tap → HOME (icons: GPS-cursor =
TRACKPAD / settings-gear / keyboard). SETTINGS is reached from HOME; the KEYS hub holds
Fn = F-keys 3x3, 123 = numpad 4x4, #$% = symbols 3x3, MOD = one-shot modifiers, and audio =
MEDIA (media moved into the hub; trackpad moved out to HOME). Trackpad exits to HOME, media
back-arrow returns to the hub. VIEW_MEDIA sits after VIEW_HUB in the enum so it (and the
trackpad, entered from HOME) never idle-times-out; only HOME/SETTINGS time out after
`TOUCH_TIMEOUT_MS` (30s).

**SETTINGS layout (3x3):** one setting per outer column — left = trackpad sensitivity, right =
display brightness. Rows 0/1 = plain +/- buttons (cell0/3 = sens +/-, cell2/5 = bright +/-;
greyed COLOR_HINT_GLYPH at the end stop: `SETTINGS_SENS_MAX` / `SETTINGS_BRIGHT_MIN/MAX`).
Row 2 = blue icon+value readout boxes, not tappable: cell6 = GPS-cursor + sens level (0..10),
cell8 = eye-open + brightness % (LVGL's built-in set has NO sun glyph — eye-open is the
stand-in; swap to LV_SYMBOL_CHARGE/TINT if preferred). Middle column: back (1) / rotate (4) /
empty (7). Cells 0-5 kept the old 2x3 numbering so the tap handler was unchanged. Sensitivity
lives in the keyboard repo's touch driver via weak hooks (`prospector_touchpad_sens_get/step`;
-1 = driver absent → the sensitivity column is simply not drawn). Volume lives on MEDIA.

**Display rotate (cell4):** `settings_toggle_rotation()` flips the ST7789V between its two
landscape orientations (ROTATED_90 ↔ ROTATED_270 via `display_set_orientation`) = 180° per tap,
so LVGL's 280x240 geometry is unchanged. It also calls `prospector_touch_set_flip()` (weak hook
into touch_input.c) to invert the touch mapping, then `lv_obj_invalidate(lv_screen_active())`
for a full redraw. 90° is deliberately not offered — it would swap to a 240x280 portrait
geometry and break every widget position.

**Navigation language:** red up-chevron = back/exit (all 8 back buttons), blue up/down
chevrons = prev/next page. Icon glyphs are LVGL's built-in FontAwesome subset in
montserrat_20 (the only compiled font); Fn/123/#$%/MOD stay text because no glyph exists.
The GPS location-arrow stands in for a mouse cursor.

**Visual system:** `GLASS_RADIUS 44` (R5.15mm at ~0.117mm/px), `UI_PAD 5` grid inset so corner
buttons clear the glass arcs, `BTN_RADIUS 14`, charcoal button fill `COLOR_BTN_BG 0x101216`,
2px accent border, +1px letter spacing. Colour roles: lilac `COLOR_ACCENT` = keys, red
`COLOR_BACK` = back, pastel blue `COLOR_PAGE` = nav / numpad operators / armed states. Named
hint greys: `COLOR_HINT 0x303030`, `COLOR_HINT_GLYPH 0x505050`, `COLOR_LANE_BG 0x0b0d10`,
`COLOR_LANE_EDGE 0x2e3238`. Armed modifiers: solid blue fill + black text (`draw_cell_on`)
plus a radius-44 blue frame around the whole screen (child object, NOT an overlay border —
a square border loses its corners to the glass; a child rounded-rect with transparent bg
avoids punching see-through holes in the opaque overlay).

**Touch dispatch contract with the keyboard repo:** `prospector_touch_tap(sx, sy)` receives
RAW rendered-screen coords (280x240) from touch_input.c and maps them per the current view's
grid (`grid_rows`/`grid_cols`: 2x3, 3x3, or 4x4). `prospector_touchpad_active()` returns true
while VIEW_TRACKPAD shows; touch_input.c then streams mouse HID instead of taps. The trackpad
view renders a scroll-lane divider at x=240 which MUST match `TP_SCROLL_ZONE_X` in
touch_input.c — one constant, two repos.

**Threading (the invariant that bit us):** three contexts exist — the touch workqueue,
the LVGL display thread (`ui_timer_cb`, 30ms), and the system workqueue. Taps arrive via a
single-slot atomic mailbox (`pending_tap`) drained on the display thread; key sends go through
an 8-deep SPSC ring (`key_ring`) drained on the system workqueue. History: key sends were
originally invoked straight from the display thread and corrupted the shared HID report
(wrong/dropped keys, varying boot to boot; consumer/volume keys use a separate report so they
masked it). Second trap, caught in review: `zmk_behavior_queue_add()` with wait=0 is NOT a
thread hop — it runs the behavior synchronously on the caller (see zmk's behavior_queue.c),
so calling it from the display thread reproduces the same race behind a msgq copy. The hop
must come from our own `k_work_submit`. One-shot mods travel inside `param1`
(`keycode | mods<<24`, ZMK implicit-mod encoding) so there's no cross-thread mod state.

**LVGL memory:** the 4x4 numpad (~32-33 objects) exhausted the OPERATOR default 20KB
`LV_Z_MEM_POOL_SIZE` → `lv_obj_create` returned NULL → crash. Pool is now 28672 in the
keyboard repo's waveshare .conf; RAM is the ceiling (32768 overflowed by ~5KB). Every LVGL
alloc here is NULL-guarded; policy on exhaustion = skip the cell, don't crash. If buttons
ever render missing, the fix is shared lv_style_t, not a bigger pool.

**Work-order history (WO numbers referenced in old commits):** WO-2 = brightness % readout on
the settings back button; WO-3 = armed-mod frame; WO-9 = home relabel MACRO→KEYS + widened
bottom row (widening later removed when labels became icons); WO-10/11 = trackpad + numpad
operator row. (The WO list lived in a since-consolidated planning doc; feature history is now
summarised in the keyboard repo's docs/information.md §12.)

### `src/brightness.c`

- `prospector_brightness_step()` / `prospector_brightness_get()` — the settings screen's
  display dimmer. Binds the display's pwm-leds controller SPECIFICALLY via
  `DEVICE_DT_GET(DT_PARENT(DT_NODELABEL(disp_bl)))`: the keyboard build defines a SECOND
  pwm-leds node (keyboard-backlight relay placeholder), and the old
  `DEVICE_DT_GET_ONE(pwm_leds)` bound whichever it found first — brightness went to a dead
  pad. Clamps 5–100% so the panel can't be turned fully off from the screen. Never touches
  the keyboard `&bl` relay (separate PWM on the halves).
- ALS fade loop (`bl_fade`) rewritten with signed maths: the upstream loop stepped a uint8_t
  and checked it `< 0` (never true — wraps to 255) and could overshoot at the bounds.
  Compiled out on the DIY dongle (`CONFIG_PROSPECTOR_USE_AMBIENT_LIGHT_SENSOR=n`); fixed for
  the day an APDS9960 is fitted.

### Everything else under `src/layouts/operator/`

Upstream OPERATOR theme (wpm meter, layer display, battery circles, output widget,
modifier indicator) — untouched except status_screen.c wiring them + the overlay on top.

## Known trade-offs (deliberate)

- Back (red ▲) and prev-page (blue ▲) share a glyph on paginated screens, colour-distinguished.
- Trackpad legend text is dim by design (0x303030) — affordance, not content.
- `GLASS_RADIUS 44` assumes R5.15mm is the active-area radius; if it's the module outline,
  tune by eye — single constant.
- Deferred left click (`TP_DTAP_MS` in touch_input.c) means 1-tap latency ≈ 180ms; the cost of
  distinguishing 1-tap-L from 2-tap-R.

## Deferred / future

- ~~Swipe-to-back gesture~~ — dropped 2026-07-06 (not wanted; chevron back buttons cover it).
- ~~Pointer acceleration curve~~ — done 2026-07-06 (threshold curve in touch_input.c,
  level adjustable from the settings screen; see the keyboard repo's CHANGES.md).
