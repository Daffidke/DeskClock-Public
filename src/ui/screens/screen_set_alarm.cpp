#include "ui/screens/screen_set_alarm.h"

#include <cstdio>
#include "display_manager.h"
#include "ui/icons.h"
#include "app_state.h"

namespace {
    constexpr int16_t HEADER_LINE_Y = 24;
    constexpr int16_t ROW_CONFIG_Y  = 50;
    constexpr int16_t ENABLED_Y       = 90;
    constexpr int16_t FOOTER_LINE_Y = 106;
    constexpr int16_t FOOTER_TEXT_Y = 123;
    constexpr int16_t FOOTER_ICON_Y = 108;
}

void ScreenSetAlarm::on_enter(const AppState& state) {
    active_field_ = EditField::HOUR;
    edit_enabled_ = state.alarm.is_enabled;
    edit_repeat_  = state.alarm.is_repeat;
    edit_hour_    = state.alarm.hour;
    edit_minute_  = state.alarm.minute;
}

InputResult ScreenSetAlarm::handle_input(ButtonId btn, ButtonEvent evt, AppState& state) {
    InputResult result;

    if (evt == ButtonEvent::LONG_PRESS && btn == ButtonId::SELECT) {
        result.target_screen = ScreenId::MAIN;
        result.refresh = RefreshType::FULL;
        return result;
    }

    if (evt != ButtonEvent::SHORT_PRESS) {
        return result;
    }

    result.refresh = RefreshType::PARTIAL;
    result.dirty_rect = DIRTY_AREA;

    if (btn == ButtonId::UP) {
        adjust_field(1);
    } else if (btn == ButtonId::DOWN) {
        adjust_field(-1);
    } else if (btn == ButtonId::SELECT) {
        const uint8_t next_field = static_cast<uint8_t>(active_field_) + 1;

        if (next_field < static_cast<uint8_t>(EditField::COUNT)) {
            active_field_ = static_cast<EditField>(next_field);
        } else {
            state.alarm.is_enabled   = edit_enabled_;
            state.alarm.is_repeat    = edit_repeat_;
            state.alarm.hour         = edit_hour_;
            state.alarm.minute       = edit_minute_;
            state.alarm.is_triggered = false;

            result.target_screen = ScreenId::MAIN;
            result.refresh = RefreshType::FULL;
        }
    }

    return result;
}

void ScreenSetAlarm::adjust_field(int delta) {
    switch (active_field_) {
        case EditField::HOUR:
            edit_hour_ = (edit_hour_ + 24 + delta) % 24;
            break;
        case EditField::MINUTE:
            edit_minute_ = (edit_minute_ + 60 + delta) % 60;
            break;
        case EditField::REPEAT:
            edit_repeat_ = !edit_repeat_;
            break;
        case EditField::STATE:
            edit_enabled_ = !edit_enabled_;
            break;
        default:
            break;
    }
}

void ScreenSetAlarm::render(const AppState&, DisplayManager& display) {
    char text_buf[16];
    int16_t bx, by;
    uint16_t bw, bh;

    // Header
    display.draw_text(5, 18, "Set Alarm");
    display.draw_line(0, HEADER_LINE_Y, display.width(), HEADER_LINE_Y);

    // Config Row
    bool is_active = (active_field_ == EditField::HOUR);
    snprintf(text_buf, sizeof(text_buf), "%02d", edit_hour_);
    display.get_text_bounds(text_buf, 48, ROW_CONFIG_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(48, ROW_CONFIG_Y, text_buf, 1, is_active);
    display.draw_text(80, ROW_CONFIG_Y, ":");

    is_active = (active_field_ == EditField::MINUTE);
    snprintf(text_buf, sizeof(text_buf), "%02d", edit_minute_);
    display.get_text_bounds(text_buf, 90, ROW_CONFIG_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(90, ROW_CONFIG_Y, text_buf, 1, is_active);

    display.draw_text(160, ROW_CONFIG_Y, "Repeat: ");
    is_active = (active_field_ == EditField::REPEAT);
    snprintf(text_buf, sizeof(text_buf), "%s", edit_repeat_ ? "Yes" : "No");
    display.get_text_bounds(text_buf, 230, ROW_CONFIG_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(230, ROW_CONFIG_Y, text_buf, 1, is_active);

    // Enable Row
    is_active = (active_field_ == EditField::STATE);
    snprintf(text_buf, sizeof(text_buf), "%s", edit_enabled_ ? "ON" : "OFF");
    display.get_text_bounds(text_buf, 130, ENABLED_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(130, ENABLED_Y, text_buf, 1, is_active);

    // Footer
    display.draw_line(0, FOOTER_LINE_Y, display.width(), FOOTER_LINE_Y);
    display.draw_bitmap(5, FOOTER_ICON_Y, Icons::up_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(28, FOOTER_TEXT_Y, "/");
    display.draw_bitmap(35, FOOTER_ICON_Y, Icons::down_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(55, FOOTER_TEXT_Y, ": Edit");
    display.draw_text(166, FOOTER_TEXT_Y, "OK: Next/Save");
}