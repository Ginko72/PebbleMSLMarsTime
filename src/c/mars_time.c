#include "pebble.h"
#include "mars_time.h"

// Unix timestamp of MSL landing epoch (Aug 5, 2012 05:17:57 UTC) — t=0 for surface time
#define MARS_EPOCH_UNIX   1344174599LL
#define MARS_LEAP_SECONDS 2LL

// Mars sol = 88775.244 Earth seconds; ratio approximation: * 1000000 / 1027491
// Error < 1 second/sol, acceptable for display
#define MARS_RATIO_NUM    1000000LL
#define MARS_RATIO_DEN    1027491LL

// SCLK calibration: at Unix 1381317960, SCLK = 434589485
// Rover clock runs fast by factor 1.000009438 → correct by * 1000000000 / 1000009438
#define SCLK_EPOCH_UNIX   1381317960LL
#define SCLK_EPOCH_VALUE  434589485LL
#define SCLK_RATE_NUM     1000000000LL
#define SCLK_RATE_DEN     1000009438LL

void sclk_from_unix(time_t unix_now, SCLK *sclk) {
    int64_t elapsed = (int64_t)unix_now - SCLK_EPOCH_UNIX;
    int64_t corrected = elapsed * SCLK_RATE_NUM / SCLK_RATE_DEN;
    sclk->sclk = SCLK_EPOCH_VALUE + corrected + MARS_LEAP_SECONDS;
}

void mars_time_from_unix(time_t unix_now, MarsTime *mt) {
    int64_t earth_sec = (int64_t)unix_now - MARS_EPOCH_UNIX + MARS_LEAP_SECONDS;
    int64_t mars_sec = earth_sec * MARS_RATIO_NUM / MARS_RATIO_DEN;

    int64_t sol_length = 24LL * 60LL * 60LL;
    mt->sol = (int32_t)(mars_sec / sol_length);

    int64_t sec_of_sol = mars_sec - (int64_t)mt->sol * sol_length;
    mt->hh = (int32_t)(sec_of_sol / 3600);
    int64_t rem = sec_of_sol - (int64_t)mt->hh * 3600;
    mt->mm = (int32_t)(rem / 60);
    mt->ss = (int32_t)(rem % 60);
}
