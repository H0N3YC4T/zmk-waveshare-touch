# Touch UI navigation map

Cell numbers are the landscape row-major grid indices. In portrait the 2x3 screens re-arrange
to 3x2 (see CHANGES.md); square grids keep their layout.

**Solid arrows** = forward navigation (label = the tapped cell). **Dotted arrows** = back
(the red ▲ / X; label = its cell). Every screen is one tap from HOME and one tap back.

```mermaid
graph TD
    NORMAL["NORMAL<br>status screen"]
    HOME["HOME<br>3x3 menu"]
    FKEYS["F-KEYS<br>3x3 · 2 pages"]
    CALC["CALC<br>5x4 · 3 pages"]
    NUMPAD["NUMPAD<br>4x4"]
    SYMBOLS["SYMBOLS<br>3x3 · 5 pages"]
    SETTINGS["SETTINGS<br>3x6"]
    TRACKPAD["TRACKPAD<br>whole screen"]
    MODIFIERS["MODIFIERS<br>2x3"]
    PAD["PAD<br>2x3"]
    MEDIA["MEDIA<br>2x3"]

    NORMAL -->|tap anywhere| HOME

    HOME -->|0| FKEYS
    HOME -->|2| NUMPAD
    HOME -->|3| SYMBOLS
    HOME -->|4| SETTINGS
    HOME -->|5| TRACKPAD
    HOME -->|6| MODIFIERS
    HOME -->|7| PAD
    HOME -->|8| MEDIA

    HOME -.->|hold 2| CALC
    HOME -.->|hold 4| BOOT[bootloader]
    CALC -.->|display tap| HOME

    HOME -.->|1| NORMAL
    FKEYS -.->|1 on page 0| HOME
    NUMPAD -.->|12| HOME
    SYMBOLS -.->|1 on page 0| HOME
    SETTINGS -.->|1| HOME
    MODIFIERS -.->|1| HOME
    PAD -.->|1| HOME
    MEDIA -.->|1| HOME
    TRACKPAD -.->|X, top-left| HOME
```

PAD is the user macro pad: up to 5 buttons (M1–M5) bound in the consuming keyboard's keymap
via a `zmk,prospector-touch-pad` node with standard binding syntax
(`bindings = <&kp LC(C) &kp LC(V) ...>;`). With nothing bound, HOME cell 7 draws greyed and
does nothing.

## What each screen does

| Screen | Grid | Cells |
| --- | --- | --- |
| HOME | 3x3 | 0 = Fn · 1 = back · 2 = 123 · 3 = #$% · 4 = settings · 5 = trackpad · 6 = MOD · 7 = PAD (keyboard icon; greyed if nothing bound) · 8 = media · **hold 2** = calculator · **hold 4** = bootloader |
| CALC | 5x4, 3 pages | on-dongle calculator (all maths local). Every page: row 0 = display (tap = exit to HOME), ⌫ top-right on pages 1-2. Page 1: digits + − × ÷, ▶ to page 2, = evaluates. Page 2: ( ) auto-pairing, %, !, decimal point, ◀ back / ▶ to binary. Page 3 (binary): 0/1, << >> & \| ~ ^, WS cycles word size 2/4/8/16, values convert between bases on page switch. Hold ⌫ = clear, hold ÷ = %. Bad input / ÷0 = Error |
| SETTINGS | 3x6 (2-col buttons) | top row = sens + (green) · back · bright + (green) · middle row = sens − (yellow) · rotate 90° CW (blue) · bright − (yellow) · bottom row = two readout boxes (not tappable: sens 0–10, brightness %) · +/− grey out at their end stops |
| MEDIA | 2x3 | 0 = vol− · 2 = vol+ · 3 = prev · 4 = play/pause · 5 = next |
| F-KEYS | 3x3 | F1–F12 over 2 pages · cell 1 = prev page (back to HOME on page 0) · cell 7 = next page (none on the last page) |
| SYMBOLS | 3x3 | 32 symbols over 5 pages, same paging as F-KEYS |
| NUMPAD | 4x4 | true HID Keypad codes (KP_*, not main-row digits): 0–9, + − * /, enter · operators + enter blue · cell 12 = back |
| MODIFIERS | 2x3 | one-shot mods: 0 = CTRL · 2 = SHFT · 3 = ALT · 5 = GUI · 4 = empty · armed = solid blue fill + black text, applied to the next key sent; cleared on leaving for NORMAL or SETTINGS |
| PAD | 2x3 | user macro pad (faces → keymap bindings): 0 = `$_` terminal · 2 = LIST task manager · 3 = WIFI browser · 4 = EYE_CLOSE show desktop · 5 = EDIT notes · 1 = back · unbound cells grey/no-op · one-shot mods do NOT apply — bake mods into the binding |
| TRACKPAD | whole screen | drag = pointer · scroll-lane drag (right strip landscape / bottom strip portrait) = scroll · 1 tap = left click · 2 taps = right click · tap-then-hold-and-drag = drag-lock · top-left X = exit |
