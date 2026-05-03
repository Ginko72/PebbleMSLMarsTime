#pragma once
#include "pebble.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // System font keys (no custom fonts yet)
    GFont font_hero;    // large Mars time HH:MM:SS
    GFont font_medium;  // UTC/local time
    GFont font_small;   // labels, sol, SCET, date

    // Row geometry
    GRect rect_sol;
    GRect rect_mars_time;
    GRect rect_scet;
    GRect rect_utc_label;
    GRect rect_utc_time;
    GRect rect_lt_label;
    GRect rect_lt_time;
    GRect rect_date;

    // Visibility flags
    bool show_scet;
    bool show_local;

    // Horizontal inset for round display bezel clearance
    int corner_inset;
} LayoutConfig;

void layout_config_init(LayoutConfig *cfg, GRect bounds);
