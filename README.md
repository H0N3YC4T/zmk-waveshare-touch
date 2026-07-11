# dongle-mk1

ZMK module for a split-keyboard dongle built from a Seeed XIAO nRF52840 and a Waveshare 1.69"
touch LCD (280x240 ST7789V, CST816S capacitive touch, glass corners ~R5.15mm). Provides the
`prospector_adapter` shield: a colour status screen plus a full touch UI — a flat 3x3 HOME
menu with F-keys, numpad, symbols, one-shot modifiers, media controls, a user macro pad,
settings (brightness, trackpad sensitivity, screen rotation), an on-dongle calculator, and a
whole-screen trackpad with mouse HID output.

## Credit

This module is derived from **[prospector-zmk-module](https://github.com/carrefinho/prospector-zmk-module)
by [carrefinho](https://github.com/carrefinho)** — the Prospector dongle concept, the adapter
shield, the OPERATOR status-screen layout, and the display driver all originate there. Go star
the original. This repo extends it with the touch UI / trackpad and trims it to just what this
hardware combination uses; the other upstream layouts (classic/radii/field) live upstream, not
here. Both repos are MIT licensed.

## Features

- **Touch UI** on top of the OPERATOR status screen: tap → HOME (3x3, every screen one tap
  away); long-press routes to per-view hold actions (hold 123 = calculator, hold settings =
  bootloader)
- **Trackpad**: whole-screen pointer with adjustable sensitivity (0–10), tap = left click,
  double-tap = right click, tap-then-hold-and-drag = drag-lock, edge scroll lane, corner exit
- **Calculator** (3 pages: basic / brackets-percent-factorial / binary-bitwise) running
  entirely on the dongle
- **Macro pad**: 5 buttons bound in the consuming keymap via a `zmk,prospector-touch-pad`
  node with standard binding syntax
- **Settings screen**: display brightness, trackpad sensitivity (with live readouts), and
  90°-step screen rotation (landscape/portrait, layouts re-arrange automatically)
- **Rotation** is pure hardware scan-out (MADCTL) + an LVGL resolution swap — no software
  rotation, no extra buffers
- CST816S gesture driver included (`touch_input.c`), gated on the DT node — the module builds
  cleanly for non-touch targets

## Usage

`config/west.yml` in your zmk-config:

```yaml
  - name: dongle-mk1
    remote: <your-remote>
    revision: <pin a commit>
```

`build.yaml` target (the dongle is the split central):

```yaml
  - board: xiao_ble//zmk
    shield: <your_keyboard_dongle_shield> prospector_adapter
```

The consuming keyboard's shield overlay declares the CST816S node (shared &i2c1, addr 0x15)
and optionally `touch_macro_0..5` macro behaviors for the media screen and a
`zmk,prospector-touch-pad` node for the macro pad. See CHANGES.md for the full design
history, tuning knobs, and the threading/architecture notes.

## License

MIT — see LICENSE. Portions copyright the ZMK Contributors and carrefinho.
