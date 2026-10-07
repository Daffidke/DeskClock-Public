#pragma once

#include <cstdint>
#include "app_types.h"
#include "services/network_service.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenSetTime {
public:
    enum class EditField : uint8_t {
        YEAR = 0,
        MONTH,
        DAY,
        HOUR,
        MINUTE,
        COUNT
    };

    static constexpr PartialBounds DIRTY_AREA = {65, 35, 150, 65};

    ScreenSetTime(NetworkService& net);

    void on_enter(AppState& state);
    RefreshType on_tick(AppState& state);
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);

private:
    void adjust_field(int delta);

    bool is_loading_;
    bool has_conn_ = false;

    EditField active_field_ = EditField::YEAR;
    NetworkService& net_;

    int16_t edit_year_   = 2026;
    uint8_t edit_month_  = 1;
    uint8_t edit_day_    = 1;
    uint8_t edit_hour_   = 0;
    uint8_t edit_minute_ = 0;
};