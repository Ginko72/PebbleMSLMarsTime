#include "layout.h"

void layout_config_init(LayoutConfig *cfg, GRect bounds) {
    int w  = bounds.size.w;
    int cy = bounds.size.h / 2;

    int h_lmst, h_medium, h_small, gap, ci;

#if PBL_DISPLAY_WIDTH >= 200
    // Large rect: emery 200×228, gabbro 200×228
    cfg->font_lmst   = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_42));
    cfg->font_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_24));
    cfg->font_small  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_18));
    cfg->font_label  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_REGULAR_18));
    h_lmst   = 48;
    h_medium = 28;
    h_small  = 22;
    gap      = 4;
    ci       = PBL_IF_ROUND_ELSE(w / 9, 0);
#elif PBL_DISPLAY_WIDTH >= 180
    // Round: chalk 180×180 — smaller label font to clear the round bezel
    cfg->font_lmst   = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_36));
    cfg->font_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_22));
    cfg->font_small  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_16));
    cfg->font_label  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_REGULAR_12));
    h_lmst   = 42;
    h_medium = 26;
    h_small  = 20;
    gap      = 3;
    ci       = w / 12;
#else
    // Small rect: aplite/basalt/diorite/flint 144×168
    cfg->font_lmst   = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_30));
    cfg->font_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_18));
    cfg->font_small  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_BOLD_14));
    cfg->font_label  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MULISH_REGULAR_14));
    h_lmst   = 34;
    h_medium = 22;
    h_small  = 18;
    gap      = 2;
    ci       = 0;
#endif

    cfg->corner_inset = ci;
    int x = ci, uw = w - 2 * ci;

    // SCET sits tight under the LMST; all other gaps use the standard gap
    int gap_scet = 1;

    // 6 rows + 5 gaps
    int total_h = h_small  + gap
                + h_lmst   + gap_scet
                + h_small  + gap
                + h_medium + gap
                + h_medium + gap
                + h_medium;

    // Chalk: SCET row is not shown — exclude it from the centering calculation
    // so the 5 visible rows sit lower and the sol line clears the round bezel.
    // The SCET rect is still computed below; its layer is NULL so nothing renders.
#if PBL_DISPLAY_WIDTH >= 180 && PBL_DISPLAY_WIDTH < 200
    int center_h = total_h - h_small - gap;
#else
    int center_h = total_h;
#endif

    int y = cy - center_h / 2;

    cfg->rect_sol       = GRect(x, y, uw, h_small);  y += h_small  + gap;
    cfg->rect_mars_time = GRect(x, y, uw, h_lmst);   y += h_lmst   + gap_scet;
    cfg->rect_scet      = GRect(x, y, uw - 12, h_small);  y += h_small  + gap;
    cfg->rect_utc       = GRect(x, y, uw, h_medium); y += h_medium + gap;
    cfg->rect_lt        = GRect(x, y, uw, h_medium); y += h_medium + gap;
    cfg->rect_date      = GRect(x, y, uw, h_medium);
}
