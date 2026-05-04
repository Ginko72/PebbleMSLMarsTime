#include "pebble.h"
#include "mars_time.h"
#include "settings.h"
#include "layout.h"
#include <string.h>

// message_keys.auto.h is empty — declare manually
extern uint32_t MESSAGE_KEY_SecondsDuration;
extern uint32_t MESSAGE_KEY_DateEU;

// String buffer sizes
#define BUF_SOL_LABEL  16   // "MSL sol 12345\0"
#define BUF_MARS_TIME  10   // "HH:MM:SS\0"
#define BUF_SCET       14   // uint32 prefix (10 digits) + "00\0" = 13 max
#define BUF_UTC        18   // "utc " + yday (5) + "T" + HH:MM + \0 = 17 max
#define BUF_LT          9   // "12:34pm\0"
#define BUF_DATE       12   // "Sun 05/03\0"

static Window      *s_window;
static LayoutConfig s_layout;
static Settings     s_settings;
static AppTimer    *s_timer = NULL;
static bool         s_active = false;

static TextLayer *s_layer_sol;
static TextLayer *s_layer_mars_time;
static TextLayer *s_layer_scet;
static TextLayer *s_layer_utc;
static TextLayer *s_layer_lt;
static TextLayer *s_layer_date;

// Static string buffers
static char s_buf_sol[BUF_SOL_LABEL];
static char s_buf_mars[BUF_MARS_TIME];
static char s_buf_scet[BUF_SCET];
static char s_buf_utc[BUF_UTC];
static char s_buf_lt[BUF_LT];
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
        uint32_t s = (uint32_t)sclk.sclk;
        snprintf(s_buf_scet, BUF_SCET, "%lu%02lu",
                 (unsigned long)(s / 100), (unsigned long)(s % 100));
    } else {
        snprintf(s_buf_mars, BUF_MARS_TIME, "%02ld:%02ld:00",
                 (long)mt.hh, (long)mt.mm);
        uint32_t s = (uint32_t)sclk.sclk;
        snprintf(s_buf_scet, BUF_SCET, "%lu00", (unsigned long)(s / 100));
    }
    text_layer_set_text(s_layer_mars_time, s_buf_mars);
    text_layer_set_text(s_layer_scet, s_buf_scet);

    // UTC day-of-year and time
    struct tm *utc = gmtime(&now);
    snprintf(s_buf_utc, BUF_UTC, "utc %dT%02d:%02d",
             (int)(uint16_t)(utc->tm_yday + 1),
             (int)(uint8_t)utc->tm_hour,
             (int)(uint8_t)utc->tm_min);
    text_layer_set_text(s_layer_utc, s_buf_utc);

    // Local time — respects watch 24h/12h setting
    struct tm *lt_now = localtime(&now);
    int lt_h = (int)(uint8_t)lt_now->tm_hour;
    if (clock_is_24h_style()) {
        snprintf(s_buf_lt, BUF_LT, "%02d:%02d", lt_h, (int)(uint8_t)lt_now->tm_min);
    } else {
        snprintf(s_buf_lt, BUF_LT, "%d:%02d%s",
                 lt_h % 12 ? lt_h % 12 : 12,
                 (int)(uint8_t)lt_now->tm_min,
                 lt_h < 12 ? "am" : "pm");
    }
    text_layer_set_text(s_layer_lt, s_buf_lt);

    // Date — MM/DD (US) or DD/MM (EU)
    strftime(s_buf_date, BUF_DATE,
             s_settings.date_eu ? "%a %d/%m" : "%a %m/%d",
             lt_now);
    text_layer_set_text(s_layer_date, s_buf_date);
}

static void prv_apply_settings(void) {
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

    t = dict_find(iter, MESSAGE_KEY_DateEU);
    if (t) {
        s_settings.date_eu = (uint8_t)(t->value->int32 != 0);
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
                                        s_layout.font_label, GTextAlignmentCenter);
    s_layer_mars_time = prv_make_layer(window, s_layout.rect_mars_time,
                                        s_layout.font_lmst, GTextAlignmentCenter);
    s_layer_scet = prv_make_layer(window, s_layout.rect_scet,
                                   s_layout.font_label, GTextAlignmentRight);
    s_layer_utc  = prv_make_layer(window, s_layout.rect_utc,
                                   s_layout.font_medium, GTextAlignmentCenter);
    s_layer_lt   = prv_make_layer(window, s_layout.rect_lt,
                                   s_layout.font_medium, GTextAlignmentCenter);
    s_layer_date = prv_make_layer(window, s_layout.rect_date,
                                   s_layout.font_label, GTextAlignmentCenter);

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
    text_layer_destroy(s_layer_scet);
    text_layer_destroy(s_layer_utc);
    text_layer_destroy(s_layer_lt);
    text_layer_destroy(s_layer_date);

    // Unload custom fonts
    fonts_unload_custom_font(s_layout.font_lmst);
    fonts_unload_custom_font(s_layout.font_medium);
    fonts_unload_custom_font(s_layout.font_small);
    fonts_unload_custom_font(s_layout.font_label);
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
