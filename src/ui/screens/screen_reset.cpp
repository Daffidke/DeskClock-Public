#include "ui/screens/screen_reset.h"

#include "services/system_service.h"
#include "display_manager.h"
#include "ui/icons.h"
#include "app_state.h"

namespace {
    constexpr int16_t HEADER_LINE_Y = 24;
    constexpr int16_t NO_X          = 95;
    constexpr int16_t YES_X         = 175;
    constexpr int16_t OPTIONS_Y     = 90;
    constexpr int16_t FOOTER_LINE_Y = 107;
    constexpr int16_t FOOTER_TEXT_Y = 123;
    constexpr int16_t FOOTER_ICON_Y = 108;
}

void ScreenReset::on_enter() {
    selected_yes_ = false;
}

InputResult ScreenReset::handle_input(ButtonId btn, ButtonEvent evt, AppState& state) {
    InputResult result;

    if (evt == ButtonEvent::LONG_PRESS && btn == ButtonId::SELECT) {
        result.target_screen = ScreenId::MAIN;
        result.refresh = RefreshType::FULL;
        return result;
    }

    if (evt != ButtonEvent::SHORT_PRESS) return result;

    switch (btn) {
        case ButtonId::UP:
        case ButtonId::DOWN:
            selected_yes_ = !selected_yes_;
            result.refresh = RefreshType::PARTIAL;
            result.dirty_rect = DIRTY_AREA;
            break;

        case ButtonId::SELECT:
            if (selected_yes_) {
                SystemService::factory_reset();
            } else {
                result.target_screen = ScreenId::MENU;
                result.refresh = RefreshType::FULL;
            }
            break;

        default:
            result.refresh = RefreshType::NONE;  
            break;
    }

    return result;
}

void ScreenReset::render(const AppState& state, DisplayManager& display) {
    int16_t bx, by;
    uint16_t bw, bh;
    
    // Header
    display.draw_text(5, 18, "Factory Reset");
    display.draw_line(0, 24, 295, 24);

    // Body
    display.draw_text(72, 44, "Reset All Settings?");
    display.draw_text(71, 62, "All data will be lost!");

    const char* active_str = selected_yes_ ? "Yes" : "No";
    const int16_t active_x = selected_yes_ ? YES_X : NO_X;

    display.get_text_bounds(active_str, active_x, OPTIONS_Y, &bx, &by, &bw, &bh);
    display.draw_highlight(bx, by, bw, bh);

    display.draw_text(NO_X, OPTIONS_Y, "No", 1, !selected_yes_);
    display.draw_text(YES_X, OPTIONS_Y, "Yes", 1, selected_yes_);

    // Footer
    display.draw_line(0, FOOTER_LINE_Y, 295, FOOTER_LINE_Y);
    display.draw_bitmap(5, FOOTER_ICON_Y, Icons::left_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(135, FOOTER_TEXT_Y, "OK");
    display.draw_bitmap(271, FOOTER_ICON_Y, Icons::right_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
}