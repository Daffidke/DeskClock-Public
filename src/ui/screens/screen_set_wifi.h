#pragma once
#include "app_types.h"
#include "services/network_service.h"

class DisplayManager;
struct AppState;

enum class ButtonId : uint8_t;
enum class ButtonEvent : uint8_t;

class ScreenSetWifi {
public:
    ScreenSetWifi(NetworkService& net);

    void on_enter(AppState& state);
    RefreshType on_tick(AppState& state);
    InputResult handle_input(ButtonId btn, ButtonEvent evt, AppState& state);
    void render(const AppState& state, DisplayManager& display);

private:
    NetworkService& net_;
    bool is_loading_;
    bool portal_active_ = false;
    bool has_conn_ = false;
};