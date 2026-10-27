/*
 * File: /src/utils.cpp
 * Project: ESP32 TramTracker
 * Created Date: 19-10-2026, 6:02:55 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 9:54:45 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "utils.h"

namespace utils {

bool same_string(const char* s1, const char* s2)
{
    return (std::strcmp(s1, s2) == 0);
}

time_t get_time(void)
{
    time_t    now;
    struct tm time_info;

    std::time(&now);
    localtime_r(&now, &time_info);

    if (time_info.tm_year <= 70) // compare year 70
    {
        return 0;
    }

    return now;
}

void configure_tz_time(const char* tz, const char* server1, const char* server2, const char* server3)
{
    setenv("TZ", tz, 1);
    tzset();

    if(sntp_enabled()) // case user want to redefine tz multiple time
    {
        sntp_stop();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL); // poll = esp32 will ask the server regulary
    sntp_setservername(0, server1);
    sntp_setservername(1, server2);
    sntp_setservername(2, server3);
    sntp_init();
}

void task_delay(const uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));   
}

} // utils