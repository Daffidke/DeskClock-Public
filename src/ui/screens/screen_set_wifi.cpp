#include "ui/screens/screen_set_wifi.h"

#include <cstdio>
#include <WiFi.h>
#include "display_manager.h"
#include "app_state.h"

namespace {
    constexpr int16_t HEADER_LINE_Y = 24;
    constexpr int16_t FOOTER_LINE_Y = 110;
}

ScreenSetWifi::ScreenSetWifi(NetworkService& net) : net_(net) {}

void ScreenSetWifi::on_enter(AppState& state) {
    is_loading_ = true;
    portal_active_ = false;
    has_conn_ = net_.connect_stored(state);
    if (!has_conn_) {
        portal_active_ = net_.start_portal(state);
    }
}

RefreshType ScreenSetWifi::on_tick(AppState& state) {
    if (is_loading_ && (state.net_state == NetworkState::IDLE || state.net_state == NetworkState::PORTAL_ACTIVE)) {
        is_loading_ = false;
        return RefreshType::FULL;
    }
    if ((portal_active_ && net_.get_stored_ssid()[0] != '\0') || state.battery < BatteryState::TWO_BAR) {
        portal_active_ = false;
        net_.stop_portal(state);
        return RefreshType::FULL;
    }
    return RefreshType::NONE;
}

InputResult ScreenSetWifi::handle_input(ButtonId btn, ButtonEvent evt, AppState& state) {
    InputResult result;

    if (evt == ButtonEvent::LONG_PRESS && btn == ButtonId::SELECT) {
        result.target_screen = ScreenId::MAIN;
        result.refresh = RefreshType::FULL;
        return result;
    }

    if (evt != ButtonEvent::SHORT_PRESS) return result;

    if (portal_active_) {
        net_.stop_portal(state);
        portal_active_ = false;
    }

    result.target_screen = ScreenId::MENU;
    result.refresh = RefreshType::FULL;
    return result;
}

void ScreenSetWifi::render(const AppState& state, DisplayManager& display) {
    char text_buf[48];

    // Header
    display.draw_text(5, 18, "Wi-Fi Setup");
    display.draw_line(0, HEADER_LINE_Y, display.width(), HEADER_LINE_Y);

    if (is_loading_) {
        display.draw_text(109, 70, "Loading...");
        return;
    }
    
    // Body
    if (!portal_active_) {
        if (net_.get_stored_ssid()[0] != '\0') {
            snprintf(text_buf, sizeof(text_buf), "Current Wi-Fi: %s", net_.get_stored_ssid());
            display.draw_text(5, 50, text_buf);
            display.draw_text(5, 80, "To change network,");
            display.draw_text(5, 100, "perform a factory reset.");
        } else if (state.battery < BatteryState::TWO_BAR) {
            display.draw_text(84, 54, "LOW BATTERY");
            display.draw_text(37, 82, "At least two bars required!");
        } else {
            display.draw_text(71, 56, "Unexpected error...");
            display.draw_text(81, 76, "Please try again.");
        }
    } else {
        display.draw_text(5, 45, "Connect to Wi-Fi AP:");
        display.draw_text(80, 65, "DeskClock-Setup:");

        snprintf(text_buf, sizeof(text_buf), "Then open: %s", WiFi.softAPIP().toString().c_str());
        display.draw_text(5, 95, text_buf);
    }

    // Footer
    display.draw_line(0, FOOTER_LINE_Y, display.width(), FOOTER_LINE_Y);
    display.draw_text(86, 125, "Return to Menu");
}