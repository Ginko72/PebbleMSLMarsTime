#include "pebble.h"
#include "settings.h"

#define KEY_SECONDS_DURATION 0
#define KEY_DATE_EU          2

void settings_load(Settings *s) {
    s->seconds_duration = persist_exists(KEY_SECONDS_DURATION)
        ? (uint8_t)persist_read_int(KEY_SECONDS_DURATION) : 30;

    s->date_eu = persist_exists(KEY_DATE_EU)
        ? (uint8_t)persist_read_int(KEY_DATE_EU) : 0;
}

void settings_save(const Settings *s) {
    persist_write_int(KEY_SECONDS_DURATION, s->seconds_duration);
    persist_write_int(KEY_DATE_EU, s->date_eu);
}
