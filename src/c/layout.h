#pragma once
#include "pebble.h"
#include <stdint.h>

typedef struct {
    GFont font_lmst;    // large Mars time HH:MM:SS
    GFont font_medium;  // UTC/local time
    GFont font_small;   // SCET
    GFont font_label;   // Mulish Regular — sol and date rows

    // Row geometry — all rows present on every platform
    GRect rect_sol;
    GRect rect_mars_time;
    GRect rect_scet;
    GRect rect_utc;
    GRect rect_lt;
    GRect rect_date;

    // Horizontal inset for round display bezel clearance
    int corner_inset;
} LayoutConfig;

void layout_config_init(LayoutConfig *cfg, GRect bounds);
