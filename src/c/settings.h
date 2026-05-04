#pragma once
#include <stdint.h>

typedef struct {
    uint8_t seconds_duration;
    uint8_t date_eu;            // 0 = MM/DD (US), 1 = DD/MM (EU)
} Settings;

void settings_load(Settings *s);
void settings_save(const Settings *s);
