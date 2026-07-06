/* The screens of the touch UI. One section per view -- its renderer (build_*) and
 * its tap handler (tap_*) side by side -- and the view_defs[] registry at the
 * bottom, which declares each view's grid, portrait handling, idle-timeout and
 * one-shot-mod policy. Navigation (touch_nav.c) only ever consults the registry. */

#include "touch_ui.h"

/* ------------------------------- NORMAL ----------------------------------- */
/* The stock status screen; the overlay is hidden. Any tap opens the menu. */

static void tap_normal(int cell) {
    ARG_UNUSED(cell);
    show_view(VIEW_HOME);
}

/* -------------------------------- HOME ------------------------------------ */
/* Icons: back / trackpad / settings / keys. Portrait: back spans the top row,
 * trackpad|settings middle, keys spans the bottom (see pmap_home). */

static void build_home(void) {
    if (ui_rot & 1) {
        draw_cell(0, 0, 2, LV_SYMBOL_UP, COLOR_BACK);
        draw_cell(1, 0, 1, LV_SYMBOL_GPS, COLOR_ACCENT);
        draw_cell(1, 1, 1, LV_SYMBOL_SETTINGS, COLOR_ACCENT);
        draw_cell(2, 0, 2, LV_SYMBOL_KEYBOARD, COLOR_ACCENT);
    } else {
        draw_cell(0, 0, 3, LV_SYMBOL_UP, COLOR_BACK);
        draw_cell(1, 0, 1, LV_SYMBOL_GPS, COLOR_ACCENT);
        draw_cell(1, 1, 1, LV_SYMBOL_SETTINGS, COLOR_ACCENT);
        draw_cell(1, 2, 1, LV_SYMBOL_KEYBOARD, COLOR_ACCENT);
    }
}

static void tap_home(int cell) {
    switch (cell) {
    case 3: show_view(VIEW_TRACKPAD); break;
    case 4: show_view(VIEW_SETTINGS); break;
    case 5: show_view(VIEW_HUB); break;
    default: show_view(VIEW_NORMAL); break; /* the whole top row is Back */
    }
}

static const uint8_t pmap_home[6] = {0, 0, 3, 4, 5, 5};

/* ------------------------------- SETTINGS --------------------------------- */
/* 3x3. One setting per outer column: left = trackpad sensitivity, right = display
 * brightness. Rows 0/1 = + and - (greyed at the end stop); row 2 = icon + value
 * readouts (blue, not keys) splitting the row 50/50. Middle: back / rotate.
 * (No sun glyph exists in LVGL's built-in set; eye-open marks brightness.) */

static void build_settings(void) {
    char lbl[20];
    int br = prospector_brightness_get();
    int sn = prospector_touchpad_sens_get();
    if (sn >= 0) {
        draw_cell(0, 0, 1, LV_SYMBOL_PLUS,
                  sn >= SETTINGS_SENS_MAX ? COLOR_HINT_GLYPH : COLOR_ACCENT);
        draw_cell(1, 0, 1, LV_SYMBOL_MINUS, sn <= 0 ? COLOR_HINT_GLYPH : COLOR_ACCENT);
    }
    draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_BACK);
    draw_cell(1, 1, 1, LV_SYMBOL_REFRESH, COLOR_PAGE); /* rotate 90deg cw per tap */
    draw_cell(0, 2, 1, LV_SYMBOL_PLUS,
              br >= SETTINGS_BRIGHT_MAX ? COLOR_HINT_GLYPH : COLOR_ACCENT);
    draw_cell(1, 2, 1, LV_SYMBOL_MINUS,
              br <= SETTINGS_BRIGHT_MIN ? COLOR_HINT_GLYPH : COLOR_ACCENT);
    /* Readout row on a temporary 2-column grid (1.5 units each); taps still
     * resolve on the 3x3 grid -- row 2 is no-op cells either way. */
    grid_cols = 2;
    if (sn >= 0) {
        lv_snprintf(lbl, sizeof(lbl), LV_SYMBOL_GPS " %d", sn);
        draw_cell(2, 0, 1, lbl, COLOR_PAGE);
    }
    lv_snprintf(lbl, sizeof(lbl), LV_SYMBOL_EYE_OPEN " %d%%", br);
    draw_cell(2, 1, 1, lbl, COLOR_PAGE);
    grid_cols = 3;
}

static void tap_settings(int cell) {
    switch (cell) {
    case 0: prospector_touchpad_sens_step(+1); build_view(VIEW_SETTINGS); break;
    case 1: show_view(VIEW_HOME); break;
    case 2: prospector_brightness_step(+BRIGHTNESS_STEP); build_view(VIEW_SETTINGS); break;
    case 3: prospector_touchpad_sens_step(-1); build_view(VIEW_SETTINGS); break;
    case 4: ui_rot = (ui_rot + 1) & 3; settings_apply_rotation(); break; /* +90deg CW */
    case 5: prospector_brightness_step(-BRIGHTNESS_STEP); build_view(VIEW_SETTINGS); break;
    default: break; /* cells 6-8: the readout row */
    }
}

/* --------------------------------- HUB ------------------------------------ */
/* Gateway to the key screens + media. No LVGL glyphs exist for F-keys or
 * modifiers, so those stay short text. */

static void build_hub(void) {
    draw_cell_l(0, "Fn", COLOR_ACCENT);
    draw_cell_l(1, LV_SYMBOL_UP, COLOR_BACK);
    draw_cell_l(2, "123", COLOR_ACCENT);
    draw_cell_l(3, "#$%", COLOR_ACCENT);
    draw_cell_l(4, LV_SYMBOL_AUDIO, COLOR_ACCENT);
    draw_cell_l(5, "MOD", COLOR_ACCENT);
}

static void tap_hub(int cell) {
    static const enum ui_view targets[6] = {
        VIEW_FKEYS, VIEW_HOME, VIEW_NUMPAD, VIEW_SYMBOLS, VIEW_MEDIA, VIEW_MODIFIERS,
    };
    if (cell >= 0 && cell < 6) {
        show_view(targets[cell]);
    }
}

/* -------------------------------- MEDIA ------------------------------------ */
/* Volume / transport, one macro per cell (defined in the consuming keyboard's
 * overlay). Cell 1 = back to the hub. */

static void build_media(void) {
    draw_cell_l(0, LV_SYMBOL_VOLUME_MID, COLOR_ACCENT);
    draw_cell_l(1, LV_SYMBOL_UP, COLOR_BACK);
    draw_cell_l(2, LV_SYMBOL_VOLUME_MAX, COLOR_ACCENT);
    draw_cell_l(3, LV_SYMBOL_PREV, COLOR_ACCENT);
    draw_cell_l(4, LV_SYMBOL_PLAY, COLOR_ACCENT);
    draw_cell_l(5, LV_SYMBOL_NEXT, COLOR_ACCENT);
}

static void tap_media(int cell) {
    static const char *const macros[6] = {
        "touch_macro_0", NULL /* back */, "touch_macro_2",
        "touch_macro_4", "touch_macro_3", "touch_macro_5",
    };
    if (cell == 1) {
        show_view(VIEW_HUB);
    } else if (cell >= 0 && cell < 6 && macros[cell] != NULL) {
        fire_macro(macros[cell]);
    }
}

/* --------------------------- FKEYS / SYMBOLS ------------------------------- */
/* Paginated 3x3 key screens: 7 keys per page; cell 1 = Back (page 0) / Prev,
 * cell 7 = Next. Key sends go through send_key (applies any armed one-shot mod). */

static const uint32_t fkeys[12] = {F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12};
static const char *const fkey_lbls[12] = {"F1", "F2", "F3",  "F4",  "F5",  "F6",
                                          "F7", "F8", "F9", "F10", "F11", "F12"};

static const uint32_t symbols[32] = {
    EXCL, AT,   HASH, DLLR, PRCNT, CARET, AMPS, STAR,
    LPAR, RPAR, MINUS, UNDER, EQUAL, PLUS, LBKT, RBKT,
    LBRC, RBRC, BSLH, PIPE, SEMI, COLON, SQT, DQT,
    COMMA, DOT, LT,  GT,  FSLH, QMARK, GRAVE, TILDE,
};
static const char *const sym_lbls[32] = {
    "!", "@", "#",  "$", "%", "^", "&", "*",
    "(", ")", "-",  "_", "=", "+", "[", "]",
    "{", "}", "\\", "|", ";", ":", "'", "\"",
    ",", ".", "<",  ">", "/", "?", "`", "~",
};

static void handle_key_page(const uint32_t *keys, int n, int cell) {
    int pages = (n + KEYS_PER_PAGE - 1) / KEYS_PER_PAGE;
    if (cell == 1) { /* Back (page 0) or Prev */
        if (cur_page == 0) {
            show_view(VIEW_HUB);
        } else {
            cur_page--;
            build_view(cur_view);
        }
        return;
    }
    if (cell == 7) { /* Next (cyclic) */
        if (pages > 1) {
            cur_page = (cur_page + 1) % pages;
            build_view(cur_view);
        }
        return;
    }
    for (int i = 0; i < KEYS_PER_PAGE; i++) {
        if (key_cells[i] == cell) {
            int idx = cur_page * KEYS_PER_PAGE + i;
            if (idx < n) {
                send_key(keys[idx]);
            }
            return;
        }
    }
}

static void build_fkeys(void) { draw_key_page(fkey_lbls, 12, cur_page); }
static void tap_fkeys(int cell) { handle_key_page(fkeys, 12, cell); }
static void build_symbols(void) { draw_key_page(sym_lbls, 32, cur_page); }
static void tap_symbols(int cell) { handle_key_page(symbols, 32, cell); }

/* -------------------------------- NUMPAD ----------------------------------- */
/* 4x4 calc grid; operators (blue) in column 3; cell 12 = Back, 14 = Enter. */

static void build_numpad(void) {
    static const char *const lbls[16] = {
        "7", "8", "9", "+",  "4", "5", "6", "-",
        "1", "2", "3", "*",  LV_SYMBOL_UP, "0", LV_SYMBOL_NEW_LINE, "/",
    };
    for (int c = 0; c < 16; c++) {
        uint32_t color = (c == 12) ? COLOR_BACK
                                   : (c % 4 == 3 || c == 14) ? COLOR_PAGE : COLOR_ACCENT;
        draw_cell(c / 4, c % 4, 1, lbls[c], color);
    }
}

static void tap_numpad(int cell) {
    /* Index 12 (Back) is 0 and handled first, so the np[cell] guard only skips
     * it -- every real key code is non-zero. */
    static const uint32_t np[16] = {N7, N8, N9, PLUS,  N4, N5, N6, MINUS,
                                    N1, N2, N3, STAR,  0,  N0, RET, FSLH};
    if (cell == 12) {
        show_view(VIEW_HUB);
    } else if (cell >= 0 && cell <= 15 && np[cell]) {
        send_key(np[cell]);
    }
}

/* ------------------------------- MODIFIERS --------------------------------- */
/* One-shot mods; armed = solid blue fill + black text, applied to the next key
 * sent (send_key). Leaving the hub family clears them (keeps_mods in show_view). */

static const uint8_t mod_bits[6] = {MOD_LCTL, 0 /* back */, MOD_LSFT,
                                    MOD_LALT, 0 /* empty */, MOD_LGUI};
static const char *const mod_lbls[6] = {"CTRL", NULL, "SHFT", "ALT", NULL, "GUI"};

static void build_modifiers(void) {
    draw_cell_l(1, LV_SYMBOL_UP, COLOR_BACK);
    for (int c = 0; c < 6; c++) {
        if (mod_lbls[c] == NULL) {
            continue;
        }
        if (pending_mods & mod_bits[c]) {
            draw_cell_on_l(c, mod_lbls[c], COLOR_PAGE);
        } else {
            draw_cell_l(c, mod_lbls[c], COLOR_ACCENT);
        }
    }
}

static void tap_modifiers(int cell) {
    if (cell == 1) {
        show_view(VIEW_HUB);
    } else if (cell >= 0 && cell < 6 && mod_bits[cell]) {
        pending_mods ^= mod_bits[cell];
        build_view(VIEW_MODIFIERS);
    }
}

/* ------------------------------- TRACKPAD ---------------------------------- */
/* Whole screen = trackpad (gestures in touch_input.c): drag = move pointer,
 * 1 tap = L-click, 2 taps = R-click, top-left corner = exit. Scroll lane =
 * logical coord >= 240 along the long axis, drawn flush to the edge: right-side
 * vertical strip in landscape, bottom horizontal strip in portrait (swipe
 * right = scroll down). */

static void build_trackpad(void) {
    lv_obj_t *lane = lv_obj_create(touch_overlay);
    if (lane != NULL) {
        if (ui_rot & 1) {
            lv_obj_set_size(lane, scr_w(), scr_h() - 240);
            lv_obj_set_pos(lane, 0, 240);
        } else {
            lv_obj_set_size(lane, scr_w() - 240, scr_h());
            lv_obj_set_pos(lane, 240, 0);
        }
        lv_obj_set_style_bg_color(lane, lv_color_hex(COLOR_LANE_BG), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(lane, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(lane, lv_color_hex(COLOR_LANE_EDGE), LV_PART_MAIN);
        lv_obj_set_style_border_width(lane, 1, LV_PART_MAIN);
        lv_obj_set_style_radius(lane, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(lane, 0, LV_PART_MAIN);
    }
    lv_obj_t *ex = lv_label_create(touch_overlay);
    if (ex != NULL) {
        lv_label_set_text(ex, LV_SYMBOL_CLOSE); /* X: exit the pad, not a nav level */
        lv_obj_set_style_text_font(ex, &lv_font_montserrat_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(ex, lv_color_hex(COLOR_BACK), LV_PART_MAIN);
        lv_obj_set_pos(ex, 16, 12); /* corner-exit affordance, clear of the glass arc */
    }
    lv_obj_t *hint = lv_label_create(touch_overlay);
    if (hint != NULL) {
        lv_label_set_text(hint, "TRACKPAD");
        lv_obj_set_style_text_font(hint, &lv_font_montserrat_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(hint, lv_color_hex(COLOR_HINT), LV_PART_MAIN);
        /* centred on the pad area (excluding the lane) */
        if (ui_rot & 1) {
            lv_obj_align(hint, LV_ALIGN_CENTER, 0, -20);
        } else {
            lv_obj_align(hint, LV_ALIGN_CENTER, -20, 0);
        }
    }
}

static void tap_trackpad(int cell) {
    /* touch_input.c only forwards the corner-exit tap here. */
    ARG_UNUSED(cell);
    show_view(VIEW_HOME);
}

/* ----------------------------- the registry -------------------------------- */

void build_view(enum ui_view v) {
    const struct view_def *d = &view_defs[v];
    lv_obj_clean(touch_overlay);
    grid_rows = d->rows;
    grid_cols = d->cols;
    if ((ui_rot & 1) && d->rearrange_2x3) {
        grid_rows = 3;
        grid_cols = 2;
    }
    if (d->build != NULL) {
        d->build();
    }

    /* Armed one-shot modifier -> blue frame visible from any key page. Drawn as its
     * own rounded-rect child (radius = glass) so it follows the screen's physical
     * corners. Cleaned up with the rest of the view. */
    if (pending_mods) {
        lv_obj_t *frame = lv_obj_create(touch_overlay);
        if (frame != NULL) {
            lv_obj_set_size(frame, scr_w(), scr_h());
            lv_obj_set_pos(frame, 0, 0);
            lv_obj_set_style_bg_opa(frame, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_color(frame, lv_color_hex(COLOR_PAGE), LV_PART_MAIN);
            lv_obj_set_style_border_width(frame, 3, LV_PART_MAIN);
            lv_obj_set_style_radius(frame, GLASS_RADIUS, LV_PART_MAIN);
            lv_obj_set_style_pad_all(frame, 0, LV_PART_MAIN);
        }
    }
}

const struct view_def view_defs[VIEW_COUNT] = {
    /*                    build             on_tap          r  c  portrait   2x3    tmout  mods */
    [VIEW_NORMAL]    = {NULL,            tap_normal,    2, 3, NULL,      false, false, false},
    [VIEW_HOME]      = {build_home,      tap_home,      2, 3, pmap_home, true,  true,  false},
    [VIEW_SETTINGS]  = {build_settings,  tap_settings,  3, 3, NULL,      false, true,  false},
    [VIEW_HUB]       = {build_hub,       tap_hub,       2, 3, p23_pos,   true,  false, true},
    [VIEW_MEDIA]     = {build_media,     tap_media,     2, 3, p23_pos,   true,  false, true},
    [VIEW_FKEYS]     = {build_fkeys,     tap_fkeys,     3, 3, NULL,      false, false, true},
    [VIEW_NUMPAD]    = {build_numpad,    tap_numpad,    4, 4, NULL,      false, false, true},
    [VIEW_SYMBOLS]   = {build_symbols,   tap_symbols,   3, 3, NULL,      false, false, true},
    [VIEW_MODIFIERS] = {build_modifiers, tap_modifiers, 2, 3, p23_pos,   true,  false, true},
    [VIEW_TRACKPAD]  = {build_trackpad,  tap_trackpad,  2, 3, NULL,      false, false, true},
};
