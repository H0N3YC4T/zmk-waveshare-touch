/* Grid geometry + button drawing for the touch UI. */

#include "touch_ui.h"

int grid_rows = 2; /* rows in the current screen's grid (2 / 3 / 4) */
int grid_cols = 3; /* cols in the current screen's grid (2 / 3 / 4) */

/* 3x3 paginated layout: 7 key cells (all but 1 = nav-top and 7 = nav-bottom). */
const int key_cells[KEYS_PER_PAGE] = {0, 2, 3, 4, 5, 6, 8};

/* One button: `pct`% of its cell width (centred), rounded, charcoal fill, accent
 * border+text; `filled` inverts (solid accent, black text) for armed/on states.
 * The grid lives inside a UI_PAD safe inset so corner buttons clear the glass arcs.
 * draw_cell() is the outline 80% default; wider variants give long labels room. */
static void draw_cell_impl(int row, int col, int w_cells, const char *text, uint32_t accent,
                           int pct, bool filled) {
    lv_coord_t cwc = (scr_w() - 2 * UI_PAD) / grid_cols;
    lv_coord_t ch = (scr_h() - 2 * UI_PAD) / grid_rows;
    lv_coord_t cw = cwc * w_cells;
    lv_coord_t bw = cw * pct / 100, bh = ch * 4 / 5;

    lv_obj_t *b = lv_obj_create(touch_overlay);
    if (b == NULL) {
        return; /* LVGL pool exhausted -- skip this cell rather than deref NULL */
    }
    lv_obj_set_size(b, bw, bh);
    lv_obj_set_pos(b, UI_PAD + col * cwc + (cw - bw) / 2, UI_PAD + row * ch + (ch - bh) / 2);
    lv_obj_set_style_bg_color(b, lv_color_hex(filled ? accent : COLOR_BTN_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(b, lv_color_hex(accent), LV_PART_MAIN);
    lv_obj_set_style_border_width(b, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(b, BTN_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_pad_all(b, 0, LV_PART_MAIN);

    lv_obj_t *l = lv_label_create(b);
    if (l == NULL) {
        return;
    }
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(l, lv_color_hex(filled ? 0x000000 : accent), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(l, 1, LV_PART_MAIN);
    lv_obj_center(l);
}

void draw_cell(int row, int col, int w_cells, const char *text, uint32_t accent) {
    draw_cell_impl(row, col, w_cells, text, accent, 80, false);
}

/* 2x3-logical cell placement for both orientations. In portrait the six cells
 * re-arrange to 3 rows x 2 cols preserving pair order: top outer pair first, then
 * back beside the centre action, then the bottom outer pair. Same table maps a
 * portrait tap back to the logical cell id the handlers use (touch_nav.c). Used
 * by every 2x3 view (HOME, HUB, MEDIA, MODIFIERS) via draw_cell_l. */
const uint8_t p23_pos[6] = {0, 2, 1, 4, 3, 5}; /* portrait pos -> logical cell */

static void draw_cell_l_impl(int lcell, const char *text, uint32_t accent, bool filled) {
    if (ui_rot & 1) {
        for (int p = 0; p < 6; p++) {
            if (p23_pos[p] == lcell) {
                draw_cell_impl(p / 2, p % 2, 1, text, accent, 80, filled);
                return;
            }
        }
    } else {
        draw_cell_impl(lcell / 3, lcell % 3, 1, text, accent, 80, filled);
    }
}
void draw_cell_l(int lcell, const char *text, uint32_t accent) {
    draw_cell_l_impl(lcell, text, accent, false);
}
void draw_cell_on_l(int lcell, const char *text, uint32_t accent) {
    draw_cell_l_impl(lcell, text, accent, true);
}

/* Draw one page of a paginated 3x3 key screen: 7 keys in cells 0,2,3,4,5,6,8;
 * cell 1 (top-middle) = Back/Prev, cell 7 (bottom-middle) = Next. */
void draw_key_page(const char *const *lbls, int n, int page) {
    int pages = (n + KEYS_PER_PAGE - 1) / KEYS_PER_PAGE;
    for (int i = 0; i < KEYS_PER_PAGE; i++) {
        int idx = page * KEYS_PER_PAGE + i;
        if (idx < n) {
            draw_cell(key_cells[i] / 3, key_cells[i] % 3, 1, lbls[idx], COLOR_ACCENT);
        }
    }
    if (page == 0) {
        draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_BACK); /* Back to hub */
    } else {
        draw_cell(0, 1, 1, LV_SYMBOL_UP, COLOR_PAGE); /* Prev page */
    }
    if (pages > 1) {
        draw_cell(2, 1, 1, LV_SYMBOL_DOWN, COLOR_PAGE); /* Next page (cell 7) */
    }
}
