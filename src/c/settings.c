#include "pebble.h"
#include "settings.h"
#include <string.h>

#define KEY_SECONDS_DURATION 0
#define KEY_TZ_LABEL         1

void settings_load(Settings *s) {
    s->seconds_duration = persist_exists(KEY_SECONDS_DURATION)
        ? (uint8_t)persist_read_int(KEY_SECONDS_DURATION) : 30;

    if (persist_exists(KEY_TZ_LABEL)) {
        persist_read_string(KEY_TZ_LABEL, s->tz_label, TZ_LABEL_MAX);
    } else {
        strncpy(s->tz_label, "lt", TZ_LABEL_MAX);
    }
}

void settings_save(const Settings *s) {
    persist_write_int(KEY_SECONDS_DURATION, s->seconds_duration);
    persist_write_string(KEY_TZ_LABEL, s->tz_label);
}
