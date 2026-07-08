# Custom HOME icons

Drop LVGL-converted icon C files in this directory and the matching HOME buttons switch
from text to the icon automatically (the icon symbols are weak — no file, no icon, the
text fallback draws instead). No CMake or code changes needed.

## Expected names

| File | Descriptor variable | HOME cell (fallback) | Status |
| --- | --- | --- | --- |
| `icon_fkeys.c` | `icon_fkeys` | 0 ("Fn") | present |
| `icon_numpad.c` | `icon_numpad` | 2 ("123") | present |
| `icon_trackpad.c` | `icon_trackpad` | 5 (GPS-cursor glyph) | present |
| `icon_modkeys.c` | `icon_modkeys` | 6 ("MOD") | text by choice |
| `icon_symbols.c` | `icon_symbols` | 3 ("#$%") | text by choice |

The variable name inside the file must match exactly (`const lv_image_dsc_t icon_trackpad`,
etc.) — it is what the weak reference in `touch_ui.h` links against.

## Making an asset

1. Draw the icon **white on transparent**, ~40x40 px (buttons are ~72x61 in the 3x3
   landscape grid; up to 48x48 fits). White matters: the UI recolors the image to the
   theme accent (`lv_obj_set_style_image_recolor`), so white pixels become theme purple —
   one asset works for every accent.
2. Convert with LVGL's image converter (https://lvgl.io/tools/imageconverter):
   **LVGL v9**, color format **ARGB8888**, output **C array**.
3. Rename the output file and the `lv_image_dsc_t` variable to the names above and drop
   the file here. `#include <lvgl.h>` at the top if the generated file lacks it.
