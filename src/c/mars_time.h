#pragma once
#include <stdint.h>
#include <time.h>

typedef struct {
    int32_t sol;
    int32_t hh;
    int32_t mm;
    int32_t ss;
} MarsTime;

typedef struct {
    int64_t sclk;
} SCLK;

void mars_time_from_unix(time_t unix_now, MarsTime *mt);
void sclk_from_unix(time_t unix_now, SCLK *sclk);
