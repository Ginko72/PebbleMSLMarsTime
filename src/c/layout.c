#include "layout.h"

void layout_config_init(LayoutConfig *cfg, GRect bounds) {
    int w = bounds.size.w;

    // System fonts — same on all platforms
    cfg->font_hero   = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    cfg->font_medium = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    cfg->font_small  = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

    // Row height constants
    int h_hero   = 52;  // BITHAM_42_BOLD line height
    int h_medium = 28;  // GOTHIC_24_BOLD line height
    int h_small  = 22;  // GOTHIC_18_BOLD line height

    // Label column width for UTC/LT rows (left side)
    int label_w = 50;

#if PBL_DISPLAY_WIDTH >= 200
    // Large rect: emery (200x228), gabbro (260x260) — all rows
    cfg->show_scet  = true;
    cfg->show_local = true;

    int ci = PBL_IF_ROUND_ELSE(w / 9, 0);
    cfg->corner_inset = ci;
    int x = ci, uw = w - 2 * ci;
    int time_x = x + label_w, time_w = uw - label_w;

    int y = 6;
    cfg->rect_sol        = GRect(x, y,  uw, h_small);   y += h_small + 4;
    cfg->rect_mars_time  = GRect(x, y,  uw, h_hero);    y += h_hero + 4;
    cfg->rect_scet       = GRect(x, y,  uw, h_small);   y += h_small + 4;
    cfg->rect_utc_label  = GRect(x,      y,  label_w, h_medium);
    cfg->rect_utc_time   = GRect(time_x, y,  time_w,  h_medium); y += h_medium + 4;
    cfg->rect_lt_label   = GRect(x,      y,  label_w, h_medium);
    cfg->rect_lt_time    = GRect(time_x, y,  time_w,  h_medium); y += h_medium + 4;
    cfg->rect_date       = GRect(x, y,  uw, h_medium);

#elif PBL_DISPLAY_WIDTH >= 180
    // Medium round: chalk (180x180) — all rows except SCET
    // Use w/12 inset (15px) so hero time has 150px usable width, matching small rect
    cfg->show_scet  = false;
    cfg->show_local = true;

    int ci = PBL_IF_ROUND_ELSE(w / 12, 0);
    cfg->corner_inset = ci;
    int x = ci, uw = w - 2 * ci;
    int time_x = x + label_w, time_w = uw - label_w;

    // Total rows: 22+50+28+28+24 = 152px content, 4 gaps of 4px = 16px → 168px
    // Center in 180px → start at y=6
    int y = 6;
    cfg->rect_sol        = GRect(x, y,  uw, h_small);   y += h_small + 4;
    cfg->rect_mars_time  = GRect(x, y,  uw, h_hero);    y += h_hero + 4;
    cfg->rect_scet       = GRect(0, 0, 0, 0);  // unused
    cfg->rect_utc_label  = GRect(x,      y,  label_w, h_medium);
    cfg->rect_utc_time   = GRect(time_x, y,  time_w,  h_medium); y += h_medium + 4;
    cfg->rect_lt_label   = GRect(x,      y,  label_w, h_medium);
    cfg->rect_lt_time    = GRect(time_x, y,  time_w,  h_medium); y += h_medium + 4;
    cfg->rect_date       = GRect(x, y,  uw, h_medium);

#else
    // Small rect: aplite/basalt/diorite/flint (144x168) — Mars, UTC, date only
    cfg->show_scet  = false;
    cfg->show_local = false;

    int ci = 0;
    cfg->corner_inset = ci;
    int x = ci, uw = w - 2 * ci;
    int time_x = x + label_w, time_w = uw - label_w;

    int y = 4;
    cfg->rect_sol        = GRect(x, y,  uw, h_small);   y += h_small + 2;
    cfg->rect_mars_time  = GRect(x, y,  uw, h_hero);    y += h_hero + 4;
    cfg->rect_scet       = GRect(0, 0, 0, 0);  // unused
    cfg->rect_utc_label  = GRect(x,      y,  label_w, h_medium);
    cfg->rect_utc_time   = GRect(time_x, y,  time_w,  h_medium); y += h_medium + 4;
    cfg->rect_lt_label   = GRect(0, 0, 0, 0);  // unused
    cfg->rect_lt_time    = GRect(0, 0, 0, 0);  // unused
    cfg->rect_date       = GRect(x, y,  uw, h_medium);
#endif
}
