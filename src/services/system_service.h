#pragma once

#include <esp_attr.h>
#include <cstdint>
#include "app_types.h"
#include "app_state.h"

class SystemService {
public:
    SystemService() = delete;

    static bool initial_wake;

    static void init_time();
    static void sync_time_rtc(DateTime& datetime);
    static bool sync_time_ntp();
    static void set_time(const DateTime& datetime);

    static void read_battery(BatteryState& battery_state);

    static bool check_alarm(const DateTime& current_time, AlarmConfig& alarm);
    static void dismiss_alarm(AlarmConfig& alarm);

    static bool tick_timer(TimerConfig& timer);
    static void reset_timer(TimerConfig& timer);

    static void factory_reset();
};