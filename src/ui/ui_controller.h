#pragma once

#include <cstdint>
#include "app_types.h"
#include "app_state.h"
#include "display_manager.h"

#include "ui/screens/screen_main.h"
#include "ui/screens/screen_weather.h"
#include "ui/screens/screen_menu.h"
#include "ui/screens/screen_set_alarm.h"
#include "ui/screens/screen_set_time.h"
#include "ui/screens/screen_set_timer.h"
#include "ui/screens/screen_set_wifi.h"
#include "ui/screens/screen_reset.h"

class UIController {
public:
    UIController(AppState& state, DisplayManager& display, NetworkService& net);

    void init();
    void change_screen(ScreenId next_screen);
    void handle_button(ButtonId btn, ButtonEvent evt);
    void on_tick();
    void update();

private:
    void request_refresh(RefreshType type, const PartialBounds& bounds = {});
    static void render_callback(void* context);
    void draw_battery_bmp();

    AppState& state_;
    DisplayManager& display_;

    static constexpr PartialBounds DIRTY_AREA = {252, 0, 44, 24};

    bool needs_render_ = false;
    bool force_full_ = false;
    PartialBounds dirty_bounds_{};

    ScreenMain     screen_main_;
    ScreenWeather  screen_weather_;
    ScreenMenu     screen_menu_;
    ScreenSetAlarm screen_set_alarm_;
    ScreenSetTime  screen_set_time_;
    ScreenSetTimer screen_set_timer_;
    ScreenReset    screen_factory_reset_;
    ScreenSetWifi  screen_set_wifi_;
};