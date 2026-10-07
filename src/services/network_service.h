#pragma once

#include <cstdint>
#include "app_types.h"
#include "app_state.h"

class NetworkService {
public:
    NetworkService() = default;

    void init(AppState& state);
    NetworkState poll(AppState& state);

    bool start_portal(AppState& state, const char* ap_ssid = "DeskClock-Setup");
    void stop_portal(AppState& state);

    bool connect_stored(AppState& state);
    void disconnect(AppState& state);

    const char* get_stored_ssid() const { return cached_ssid_; }

private:
    char cached_ssid_[33] = {0};
    uint32_t connect_start_ms_ = 0;
    uint32_t portal_transition_ms_ = 0;
    bool portal_stop_pending_ = false;

    bool fetch_ip_location(LocationData& loc);
    void load_stored_location(LocationData& loc);
    bool fetch_weather_forecast(WeatherData& weather);
};