#include "pebble.h"
#include "mars_time.h"
#include "settings.h"
#include "layout.h"
#include <string.h>

// message_keys.auto.h is empty — declare manually
extern uint32_t MESSAGE_KEY_SecondsDuration;
extern uint32_t MESSAGE_KEY_TZLabel;

// String buffer sizes
#define BUF_SOL_LABEL  16   // "MSL sol 12345\0"
#define BUF_MARS_TIME  10   // "HH:MM:SS\0"
#define BUF_SCET       12   // "831084047\0"
#define BUF_UTC_TIME   12   // "123T13:51\0"
#define BUF_LT_TIME     6   // "13:51\0"
#define BUF_DATE       12   // "Sun 05/03\0"

static Window      *s_window;
static LayoutConfig s_layout;
static Settings     s_settings;
static AppTimer    *s_timer = NULL;
static bool         s_active = false;

// TextLayers — SCET and LT layers are NULL when not shown on this platform
static TextLayer *s_layer_sol;
static TextLayer *s_layer_mars_time;
static TextLayer *s_layer_scet;
static TextLayer *s_layer_utc_label;
static TextLayer *s_layer_utc_time;
static TextLayer *s_layer_lt_label;
static TextLayer *s_layer_lt_time;
static TextLayer *s_layer_date;

// Static string buffers
static char s_buf_sol[BUF_SOL_LABEL];
static char s_buf_mars[BUF_MARS_TIME];
static char s_buf_scet[BUF_SCET];
static char s_buf_utc[BUF_UTC_TIME];
static char s_buf_lt[BUF_LT_TIME];
static char s_buf_date[BUF_DATE];

static void prv_update_display(void) {
    time_t now = time(NULL);

    MarsTime mt;
    mars_time_from_unix(now, &mt);

    SCLK sclk;
    sclk_from_unix(now, &sclk);

    // Sol label
    snprintf(s_buf_sol, BUF_SOL_LABEL, "MSL sol %ld", (long)mt.sol);
    text_layer_set_text(s_layer_sol, s_buf_sol);

    // Mars time: show seconds only when active
    if (s_active) {
        snprintf(s_buf_mars, BUF_MARS_TIME, "%02ld:%02ld:%02ld",
                 (long)mt.hh, (long)mt.mm, (long)mt.ss);
        if (s_layer_scet) {
            int64_t prefix = sclk.sclk / 100;
            int64_t suffix = sclk.sclk % 100;
            snprintf(s_buf_scet, BUF_SCET, "%lld%02lld", (long long)prefix, (long long)suffix);
        }
    } else {
        snprintf(s_buf_mars, BUF_MARS_TIME, "%02ld:%02ld:--",
                 (long)mt.hh, (long)mt.mm);
        if (s_layer_scet) {
            int64_t prefix = sclk.sclk / 100;
            snprintf(s_buf_scet, BUF_SCET, "%lld--", (long long)prefix);
        }
    }
    text_layer_set_text(s_layer_mars_time, s_buf_mars);
    if (s_layer_scet) {
        text_layer_set_text(s_layer_scet, s_buf_scet);
    }

    // UTC day-of-year and time
    struct tm *utc = gmtime(&now);
    snprintf(s_buf_utc, BUF_UTC_TIME, "%dT%02d:%02d",
             utc->tm_yday + 1, utc->tm_hour, utc->tm_min);
    text_layer_set_text(s_layer_utc_label, "UTC");
    text_layer_set_text(s_layer_utc_time, s_buf_utc);

    // Local time
    if (s_layer_lt_time) {
        struct tm *lt = localtime(&now);
        snprintf(s_buf_lt, BUF_LT_TIME, "%02d:%02d", lt->tm_hour, lt->tm_min);
        text_layer_set_text(s_layer_lt_label, s_settings.tz_label);
        text_layer_set_text(s_layer_lt_time, s_buf_lt);
    }

    // Date
    struct tm *lt = localtime(&now);
    strftime(s_buf_date, BUF_DATE, "%a %m/%d", lt);
    text_layer_set_text(s_layer_date, s_buf_date);
}

static void prv_apply_settings(void) {
    if (s_layer_lt_label) {
        text_layer_set_text(s_layer_lt_label, s_settings.tz_label);
    }
    prv_update_display();
}

static void prv_timer_callback(void *context) {
    s_active = false;
    s_timer  = NULL;
    tick_timer_service_unsubscribe();
    tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler)(void *)context);
    prv_update_display();
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed);

static void prv_accel_tap_handler(AccelAxisType axis, int32_t direction) {
    light_enable_interaction();

    uint32_t duration_ms = (uint32_t)s_settings.seconds_duration * 1000;

    if (s_active) {
        app_timer_reschedule(s_timer, duration_ms);
        return;
    }

    s_active = true;
    tick_timer_service_unsubscribe();
    tick_timer_service_subscribe(SECOND_UNIT, prv_tick_handler);
    s_timer = app_timer_register(duration_ms, prv_timer_callback,
                                 (void *)prv_tick_handler);
    prv_update_display();
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    prv_update_display();
}

static void prv_inbox_received(DictionaryIterator *iter, void *context) {
    Tuple *t;

    t = dict_find(iter, MESSAGE_KEY_SecondsDuration);
    if (t) {
        s_settings.seconds_duration = (uint8_t)t->value->int32;
    }

    t = dict_find(iter, MESSAGE_KEY_TZLabel);
    if (t) {
        strncpy(s_settings.tz_label, t->value->cstring, TZ_LABEL_MAX - 1);
        s_settings.tz_label[TZ_LABEL_MAX - 1] = '\0';
    }

    settings_save(&s_settings);
    prv_apply_settings();
}

static TextLayer *prv_make_layer(Window *window, GRect rect, GFont font,
                                  GTextAlignment align) {
    TextLayer *layer = text_layer_create(rect);
    text_layer_set_background_color(layer, GColorClear);
    text_layer_set_text_color(layer, GColorWhite);
    text_layer_set_font(layer, font);
    text_layer_set_text_alignment(layer, align);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(layer));
    return layer;
}

static void main_window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root);

    layout_config_init(&s_layout, bounds);

    s_layer_sol       = prv_make_layer(window, s_layout.rect_sol,
                                        s_layout.font_small, GTextAlignmentCenter);
    s_layer_mars_time = prv_make_layer(window, s_layout.rect_mars_time,
                                        s_layout.font_hero, GTextAlignmentCenter);
    s_layer_utc_label = prv_make_layer(window, s_layout.rect_utc_label,
                                        s_layout.font_small, GTextAlignmentLeft);
    s_layer_utc_time  = prv_make_layer(window, s_layout.rect_utc_time,
                                        s_layout.font_medium, GTextAlignmentRight);
    s_layer_date      = prv_make_layer(window, s_layout.rect_date,
                                        s_layout.font_medium, GTextAlignmentCenter);

    if (s_layout.show_scet) {
        s_layer_scet = prv_make_layer(window, s_layout.rect_scet,
                                       s_layout.font_small, GTextAlignmentCenter);
    } else {
        s_layer_scet = NULL;
    }

    if (s_layout.show_local) {
        s_layer_lt_label = prv_make_layer(window, s_layout.rect_lt_label,
                                           s_layout.font_small, GTextAlignmentLeft);
        s_layer_lt_time  = prv_make_layer(window, s_layout.rect_lt_time,
                                           s_layout.font_medium, GTextAlignmentRight);
    } else {
        s_layer_lt_label = NULL;
        s_layer_lt_time  = NULL;
    }

    tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
    accel_tap_service_subscribe(prv_accel_tap_handler);

    prv_update_display();
}

static void main_window_unload(Window *window) {
    animation_unschedule_all();

    if (s_active && s_timer) {
        app_timer_cancel(s_timer);
        s_timer = NULL;
    }

    accel_tap_service_unsubscribe();
    tick_timer_service_unsubscribe();

    text_layer_destroy(s_layer_sol);
    text_layer_destroy(s_layer_mars_time);
    text_layer_destroy(s_layer_utc_label);
    text_layer_destroy(s_layer_utc_time);
    text_layer_destroy(s_layer_date);
    if (s_layer_scet)     text_layer_destroy(s_layer_scet);
    if (s_layer_lt_label) text_layer_destroy(s_layer_lt_label);
    if (s_layer_lt_time)  text_layer_destroy(s_layer_lt_time);
}

int main(void) {
    settings_load(&s_settings);

    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load   = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(s_window, true);

    app_message_register_inbox_received(prv_inbox_received);
    app_message_open(128, 0);

    app_event_loop();

    window_destroy(s_window);
    return 0;
}
