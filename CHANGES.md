# Change ledger — zmk-waveshare-touch

Everything this module adds on top of carrefinho/prospector-zmk-module (from which it was
extracted 2026-07-06, having lived as a fork branch until then), in one place. This file
carries the detailed rationale and history that the code comments deliberately don't, so future
edits have full context. Companion docs live in the keyboard repo
(Keyboard-Prototype_Mk1: docs/information.md + docs/issues.md).

Consumed by the keyboard repo via an exact-SHA pin in `config/west.yml` — every change here
needs a commit + push + pin bump there. Hardware: Seeed XIAO nRF52840 + Waveshare 1.69" LCD
(280x240, ST7789V, glass corners R5.15mm ≈ 44px) + CST816S touch, OPERATOR layout.
The CST816S gesture driver (`touch_input.c`) lives HERE now (adapter `src/`), moved in from
the keyboard repo — both sides of every touch seam are in this repo.

## Calculator correctness fixes (2026-07-11)

- **Engine float -> double.** The FPv4-SP FPU is single-precision, but a calculator waits on
  finger taps -- softfloat doubles cost nothing perceptible and lift exact-integer range from
  2^24 (~16.7M, where 99999999 displayed as 100000000) to 2^53.
- **Strict end-of-parse.** All four parse sites now go through `calc_run_parser()`, which flags
  unconsumed trailing input as an error -- malformed expressions show `Error` instead of
  silently returning the partial result (e.g. `5<2` used to display `5`).
- **Shift buttons push both chars.** `<<`/`>>` inserted a single `<`/`>` per tap while the
  parser reads shifts as two chars, so one tap gave a silently wrong answer (via the bug
  above). One tap now inserts the pair, and backspace deletes a shift as one keystroke.

## Long-press + on-dongle calculator (2026-07-09)

**Long-press routing.** The touch driver now flags a press held >= `TOUCH_HOLD_MS` (700ms) as a
hold: `prospector_touch_tap()` gained a `hold` arg, carried through the tap mailbox on a spare
bit, and `view_defs[]` gained an `on_hold(cell)` slot. A view with no `on_hold` treats holds as
taps (harmless everywhere), so only HOME opts in. On HOME: **hold 123 -> CALC**, **hold settings
-> the dongle's bootloader** (fires ZMK's built-in `bootloader` reset behavior by its DT name via
the existing key-ring path, so it does the right reset for whatever bootloader the board uses --
no board-specific magic in this module). Other HOME cells fall through to their normal tap.

**CALC** (`VIEW_CALC`) is a self-contained calculator -- all maths on the XIAO's M4F, the host is
never touched. 5x4 grid: row 0 is the display (spans all four columns; a tap there exits to
HOME), rows 1-4 mirror the numpad except bottom-left back -> backspace and enter -> `=`
(evaluate). Integer input, `+ - * /` with proper precedence via a small recursive-descent
evaluator (`calc_expr`/`calc_term`/`calc_num` over the display string), doubles on the FPU,
result formatted with `snprintf("%.6g")` (picolibc double-printf is enabled in this build).
Divide-by-zero / malformed input shows `Error`; after a result a digit restarts and an operator
continues from it. No parentheses, no unary minus, no decimal-point key (results can still be
fractional). Needs `CONFIG_LV_USE_IMAGE`-independent label rendering only.

## Flatten the menu: HOME goes 3x3, the HUB sub-menu is gone (2026-07-08)

Every screen is now one tap from HOME and one tap back. HOME (2x3) + HUB (2x3) collapse into a
single 3x3 HOME: `0` Fn (F-keys), `1` back, `2` 123 (numpad), `3` #$% (symbols), `4` settings,
`5` trackpad, `6` MOD (one-shot modifiers), `7` reserved (greyed keyboard icon, no-op --
placeholder for a future programmable pad), `8` media. All back buttons that previously
returned to the hub (F-keys/symbols page 0, modifiers) now return to HOME; media and numpad
already did.

Because HOME now sits between MODIFIERS and the key screens, it keeps armed one-shot mods
(`keeps_mods` flipped to true in view_defs) -- arming Ctrl then crossing HOME to SYMBOLS keeps
the mod, exactly as crossing the hub used to. Mods still clear on reaching NORMAL or SETTINGS.
As a 3x3, HOME no longer uses the p23_pos portrait re-arrangement (square grids keep their
layout in portrait, like SETTINGS).

Three constants existed on both sides of a seam, kept in sync only by "must match/track"
comments. All hoisted to one definition each in `touch_ui.h`:
- `TP_SCROLL_ZONE` (240): was a define in touch_input.c + bare `240` literals where
  build_trackpad draws the lane divider. Now defined once in touch_ui.h; both files use it.
- `TP_SENS_MAX`: now an alias of `SETTINGS_SENS_MAX` (the settings UI's end-stop grey-out).
- brightness.c's 5..100% clamp: now literally `SETTINGS_BRIGHT_MIN/MAX`.

touch_input.c gained `#include "touch_ui.h"` for this (it implements four `prospector_*` hooks
the header declares, so the include also buys prototype checking it never had); brightness.c
likewise. Same-repo includes only became possible when touch_input.c moved in from the keyboard
repo at extraction (2026-07-06) — the "one constant, two repos" era is over. Also corrected two
stale CHANGES.md passages (the rotation section still described the pre-4-step 180° flip;
`draw_cell_on` → `draw_cell_on_l`) and two landscape-only scroll-lane comments in touch_input.c.

**Tap-then-hold-and-drag** = drag-lock: left button held for the duration of the drag, released
on lift. This is the standard trackpad "drag" gesture (tap once, then touch down again and hold
while moving, rather than needing to hold the button the whole time). Mechanically it's a second
touch inside the existing double-tap window (`TP_DTAP_MS`) that *moves* past the dead-zone
instead of releasing quickly:
- If the 2nd touch releases quickly (a normal tap) -> unchanged, still resolves as the right-click.
- If it moves first -> commits to a new `TP_DRAG` mode instead of `TP_MOTION`: presses and holds
  MB1 (`tp_drag_cmd` atomic, drained in `tp_work_handler`), then shares the exact same motion-
  accumulation code as `TP_MOTION` (the `tp_mode == TP_MOTION || tp_mode == TP_DRAG` branch) --
  the only difference is the held button. Release fires on finger-lift.
- `tp_drag_candidate` is captured from `tp_first_tap`'s value at touch-down and doesn't touch
  `tp_first_tap` itself, so the existing right-click-at-quick-release path is untouched.
- The deferred single-tap-fallback timer (`tp_tap_work`) is cancelled as soon as this second touch
  begins (not just at its eventual resolution) -- otherwise a user who taps, touches down again,
  and *pauses* longer than `TP_DTAP_MS` before starting to move would get a stray left-click
  fired mid-hold, underneath what's about to become a drag.
- No new constants: reuses `TP_MOVE_DEADZONE_PX` (same tap-vs-drag threshold as a first touch)
  and `TP_DTAP_MS` (same double-tap window). No positional constraint on the 2nd touch, matching
  the existing double-tap-right-click (only timing gates it, not location).

## HOME menu: 6 discrete buttons + true numpad HID (2026-07-08)

**HOME** dropped its two spanning buttons (back across the top row, keys across the bottom in
portrait) for 6 plain single-cell buttons, reusing the shared `p23_pos` portrait re-arrangement
that HUB/MEDIA/MODIFIERS already used (the bespoke `pmap_home` table + `build_home`'s
landscape/portrait branch are gone). New layout: `0` media, `1` back, `2` numpad ("123"),
`3` keyboard sub-menu (the hub), `4` settings, `5` trackpad. Media and numpad are now reachable
directly from HOME as well as through the hub (their hub shortcuts are unchanged); both screens'
back cell returns to HOME either way, matching how trackpad already worked — so a hub->numpad->
back trip lands on HOME rather than back on the hub. Simple and consistent, at the cost of that
one breadcrumb skip; revisit if it's confusing in practice.

**NUMPAD now sends true HID Keypad codes** (`KP_N0..KP_N9`, `KP_PLUS/MINUS/MULTIPLY/DIVIDE`,
`KP_ENTER`) instead of the main-row digit/shifted-symbol codes (`N0..N9`, `PLUS`, `MINUS`, `STAR`,
`FSLH`, `RET`) it sent before. Same rendered labels, different HID usage codes underneath — this
matters for apps/fields that distinguish numpad input from top-row digits (numeric entry fields,
spreadsheets, some RDP/game clients with Num Lock-aware bindings).

## NORMAL status screen (2026-07-07, corrected 2026-07-08)

**Modifier row font:** no custom (FoundryGridnik/DINish) font ships smaller than `FG_Medium_20`
(20px) — checked every font ever carried by this module's history, nothing smaller ever
existed. Switched the modifier indicator (`modifier_indicator.c`) to `lv_font_montserrat_16`,
an LVGL built-in (`CONFIG_LV_FONT_MONTSERRAT_16=y` in `prospector_adapter.conf`) — no new font
asset needed, but it's a different type family from the FoundryGridnik/DINish fonts used by the
rest of the NORMAL screen. Revisit if that reads as inconsistent on hardware.

**Modifier row width tightened, portrait only — 230 -> 200 (2026-07-08).** The row uses
`LV_FLEX_ALIGN_SPACE_BETWEEN`, which stretches the gaps between the 4 mod labels + 3
separators to fill the ENTIRE declared container width — so that number is the row's real
on-screen footprint (the margins between mod keys), not just a bounding box. At 230, centred,
portrait had only 5px of clearance to the screen edge on each side ((240-230)/2); the montserrat
font swap changed glyph metrics enough that this was reported as the row's width not
comfortably fitting. Landscape was fine as-is and stays untouched at 230/x=25.

Fix: `modifier_indicator.c` gained `zmk_widget_modifier_indicator_set_width()` (the row is a
flex container with SPACE_BETWEEN, so changing the parent width alone re-spaces the gaps — no
child repositioning needed, unlike wpm_meter/layer_display's absolute-positioned children).
`status_screen_reflow()` calls it per-orientation: 200 in portrait (20px clearance each side,
x=20), 230 in landscape (unchanged, x=25). First attempt at this fix mistakenly changed
landscape too (40/230->200 uniformly) — corrected to portrait-only per user feedback.

**Battery moved under the output (USB/BLE + profile slots) widget — portrait only.** The
original attempt (2026-07-07) shrank the WPM meter, battery, and output widgets in BOTH
orientations to force the stacked arrangement to also fit landscape; that shrink was not
requested and was reverted on 2026-07-08 (widgets are back to their original full sizes:
`WPM_BAR_HEIGHT` 90, battery `arc_size` 58 / widget 62, output buttons/slots 29 / widget 62).

Reverting the shrink means landscape genuinely cannot stack battery under output: mod(24) +
wpm(90) + layer(6) + output(62) + battery(62) alone totals 244px, already past the 240px screen
height with zero gaps between them. **Landscape keeps its original side-by-side layout**
(battery left, output right, unchanged from before any of this). **Portrait** (280px tall) had
slack to spare all along and needed no shrink — battery sits under output there at full size,
reusing the same two vertical slots (152/216) the widgets occupied before, just swapped which
widget is in which slot.

## Files changed vs upstream

### `src/touch/` + `src/status_screen.c` — the entire touch UI (biggest change)

(2026-07-07: the `layouts/operator/` tree was flattened — touch UI now lives in `src/touch/`
one feature per file, widgets in `src/widgets/`, entry point at `src/status_screen.c`; the
`custom_status_screen.c` #include indirection and the single-entry layout glob are gone.)

**Views (menu tree, flat since 2026-07-08):** NORMAL → tap → HOME (3x3), and every screen is
one tap from HOME: Fn = F-keys 3x3 paged, 123 = numpad 4x4, #$% = symbols 3x3 paged,
settings-gear = SETTINGS, GPS-cursor = TRACKPAD, MOD = one-shot modifiers, audio = MEDIA,
cell 7 reserved (greyed keyboard icon, no-op). Every back returns to HOME (trackpad via the
top-left X). Idle timeout is declared per view in view_defs[] (not enum order): only
HOME/SETTINGS return to NORMAL after `TOUCH_TIMEOUT_MS` (30s); key screens, media and
trackpad never time out -- exit is always explicit. See docs/Touch-UI.md for the map.

**SETTINGS layout (3x3):** one setting per outer column — left = trackpad sensitivity, right =
display brightness. Rows 0/1 = plain +/- buttons (cell0/3 = sens +/-, cell2/5 = bright +/-;
greyed COLOR_GREY at the end stop: `SETTINGS_SENS_MAX` / `SETTINGS_BRIGHT_MIN/MAX`).
Row 2 = blue icon+value readout boxes, not tappable: cell6 = GPS-cursor + sens level (0..10),
cell8 = eye-open + brightness % (LVGL's built-in set has NO sun glyph — eye-open is the
stand-in; swap to LV_SYMBOL_CHARGE/TINT if preferred). Middle column: back (1) / rotate (4) /
empty (7). Cells 0-5 kept the old 2x3 numbering so the tap handler was unchanged. Sensitivity
lives in the keyboard repo's touch driver via weak hooks (`prospector_touchpad_sens_get/step`;
-1 = driver absent → the sensitivity column is simply not drawn). Volume lives on MEDIA.

**Display rotate (cell4):** `settings_apply_rotation()` (`touch_rotation.c`) steps the ST7789V
through all four orientations 90° CW per tap — `ui_rot` 0..3 mapped through `rot_to_panel[]` to
`display_set_orientation`, a pure MADCTL scan-out change. LVGL's logical resolution swaps
280x240 landscape ↔ 240x280 portrait; `prospector_touch_set_orientation()` keeps the touch
transform in sync, then `status_screen_reflow()` + `build_view()` re-lay the NORMAL screen and
the current touch screen for the new dimensions. Four taps = full circle. (This section
previously described the original 180°-only version — `settings_toggle_rotation()` /
`prospector_touch_set_flip()` — which the 4-step rework of 2026-07-06 replaced; corrected
2026-07-08.)

**Navigation language:** red up-chevron = back/exit (all 8 back buttons), blue up/down
chevrons = prev/next page. Icon glyphs are LVGL's built-in FontAwesome subset in
montserrat_20 (the only compiled font); Fn/123/#$%/MOD stay text because no glyph exists.
The GPS location-arrow stands in for a mouse cursor.

**Visual system:** `GLASS_RADIUS 44` (R5.15mm at ~0.117mm/px), `UI_PAD 5` grid inset so corner
buttons clear the glass arcs, `BTN_RADIUS 14`, charcoal button fill `COLOR_CHARCOAL 0x101216`,
2px accent border, +1px letter spacing. Colour roles: lilac `COLOR_PURPLE` = keys, red
`COLOR_RED` = back, pastel blue `COLOR_BLUE` = nav / numpad operators / armed states. Named
hint greys: `COLOR_DARK_GREY 0x303030`, `COLOR_GREY 0x505050`, `COLOR_NEAR_BLACK 0x0b0d10`,
`COLOR_SLATE 0x2e3238`. Armed modifiers: solid blue fill + black text (`draw_cell_on_l`)
plus a radius-44 blue frame around the whole screen (child object, NOT an overlay border —
a square border loses its corners to the glass; a child rounded-rect with transparent bg
avoids punching see-through holes in the opaque overlay).

**Touch dispatch contract (both sides in this repo since the 2026-07-06 extraction):**
`prospector_touch_tap(sx, sy)` receives logical-screen coords (280x240 landscape / 240x280
portrait, per the current rotation) from touch_input.c and maps them per the current view's
grid (`grid_rows`/`grid_cols`: 2x3, 3x3, or 4x4). `prospector_touchpad_active()` returns true
while VIEW_TRACKPAD shows; touch_input.c then streams mouse HID instead of taps. The trackpad
scroll-lane boundary is `TP_SCROLL_ZONE` in touch_ui.h — ONE shared constant (hoisted
2026-07-08; it was previously duplicated as a define in touch_input.c plus bare literals in
build_trackpad, with a MUST-match comment doing the work a compiler should).

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

### Everything else under `src/widgets/`

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
