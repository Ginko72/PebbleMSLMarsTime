#pragma once
#include <stdint.h>

#define TZ_LABEL_MAX 8

typedef struct {
    uint8_t seconds_duration;
    char    tz_label[TZ_LABEL_MAX];
} Settings;

void settings_load(Settings *s);
void settings_save(const Settings *s);
