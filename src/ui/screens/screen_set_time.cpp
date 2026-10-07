#include "ui/screens/screen_set_time.h"

#include <cstdio>
#include "services/system_service.h"
#include "display_manager.h"
#include "ui/icons.h"
#include "app_state.h"

namespace {
    constexpr int16_t HEADER_LINE_Y = 24;
    constexpr int16_t ROW_DATE_Y    = 60;
    constexpr int16_t ROW_TIME_Y    = 92;
    constexpr int16_t FOOTER_LINE_Y = 106;
    constexpr int16_t FOOTER_TEXT_Y = 123;
    constexpr int16_t FOOTER_ICON_Y = 108;

    uint8_t get_days_in_month(int16_t year, uint8_t month) {
        if (month == 2) {
            const bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            return is_leap ? 29 : 28;
        }
        if (month == 4 || month == 6 || month == 9 || month == 11) {
            return 30;
        }
        return 31;
    }
}

ScreenSetTime::ScreenSetTime(NetworkService& net) : net_(net) {}

void ScreenSetTime::on_enter(AppState& state) {
    is_loading_ = true;
    has_conn_ = net_.connect_stored(state);
    if (has_conn_) return;

    active_field_ = EditField::YEAR;

    edit_year_   = state.time.year;
    edit_month_  = state.time.month;
    edit_day_    = state.time.day;
    edit_hour_   = state.time.hour;
    edit_minute_ = state.time.minute;
}

RefreshType ScreenSetTime::on_tick(AppState& state) {
    if (is_loading_ && state.net_state == NetworkState::IDLE) {
        is_loading_ = false;
        return RefreshType::FULL;
    }
    return RefreshType::NONE;
}

InputResult ScreenSetTime::handle_input(ButtonId btn, ButtonEvent evt, AppState& state) {
    InputResult result;

    if ((evt == ButtonEvent::LONG_PRESS && btn == ButtonId::SELECT) || has_conn_) {
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
            state.time.year   = edit_year_;
            state.time.month  = edit_month_;
            state.time.day    = edit_day_;
            state.time.hour   = edit_hour_;
            state.time.minute = edit_minute_;
            state.time.second = 0;

            SystemService::set_time(state.time);

            result.target_screen = ScreenId::MAIN;
            result.refresh = RefreshType::FULL;
        }
    }

    return result;
}


void ScreenSetTime::adjust_field(int delta) {
    switch (active_field_) {
        case EditField::YEAR:
            edit_year_ += delta;
            if (edit_year_ < 2024) edit_year_ = 2024;
            if (edit_year_ > 2099) edit_year_ = 2099;
            break;

        case EditField::MONTH: {
            int m = edit_month_ + delta;
            if (m < 1) m = 12;
            else if (m > 12) m = 1;
            edit_month_ = m;

            const uint8_t max_days = get_days_in_month(edit_year_, edit_month_);
            if (edit_day_ > max_days) edit_day_ = max_days;
            break;
        }

        case EditField::DAY: {
            const uint8_t max_days = get_days_in_month(edit_year_, edit_month_);
            int d = edit_day_ + delta;
            if (d < 1) d = max_days;
            else if (d > max_days) d = 1;
            edit_day_ = d;
            break;
        }

        case EditField::HOUR: {
            int h = edit_hour_ + delta;
            if (h < 0) h = 23;
            else if (h > 23) h = 0;
            edit_hour_ = h;
            break;
        }

        case EditField::MINUTE: {
            int min = edit_minute_ + delta;
            if (min < 0) min = 59;
            else if (min > 59) min = 0;
            edit_minute_ = min;
            break;
        }

        default:
            break;
    }
}

void ScreenSetTime::render(const AppState&, DisplayManager& display) {
    char text_buf[16];
    int16_t bx, by;
    uint16_t bw, bh;

    // Header
    display.draw_text(5, 18, "Set Date and Time");
    display.draw_line(0, HEADER_LINE_Y, display.width(), HEADER_LINE_Y);

    if (is_loading_) {
        display.draw_text(109, 70, "Loading...");
        return;
    }
    if (has_conn_) {
        display.draw_text(42, 59, "Time successfully updated");
        display.draw_text(56, 80, "automatically via Wi-Fi.");
        display.draw_line(0, FOOTER_LINE_Y, display.width(), FOOTER_LINE_Y);
        display.draw_text(121, FOOTER_TEXT_Y, "Return");
        return;
    }

    // Date Row
    bool is_active = (active_field_ == EditField::YEAR);
    snprintf(text_buf, sizeof(text_buf), "%04d", edit_year_);
    display.get_text_bounds(text_buf, 76, ROW_DATE_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(76, ROW_DATE_Y, text_buf, 1, is_active);
    display.draw_text(128, ROW_DATE_Y, "-");

    is_active = (active_field_ == EditField::MONTH);
    snprintf(text_buf, sizeof(text_buf), "%02d", edit_month_);
    display.get_text_bounds(text_buf, 142, ROW_DATE_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(142, ROW_DATE_Y, text_buf, 1, is_active);
    display.draw_text(172, ROW_DATE_Y, "-");

    is_active = (active_field_ == EditField::DAY);
    snprintf(text_buf, sizeof(text_buf), "%02d", edit_day_);
    display.get_text_bounds(text_buf, 186, ROW_DATE_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(186, ROW_DATE_Y, text_buf, 1, is_active);

    // Time Row
    is_active = (active_field_ == EditField::HOUR);
    snprintf(text_buf, sizeof(text_buf), "%02d", edit_hour_);
    display.get_text_bounds(text_buf, 116, ROW_TIME_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(116, ROW_TIME_Y, text_buf, 1, is_active);
    display.draw_text(146, ROW_TIME_Y, ":");

    is_active = (active_field_ == EditField::MINUTE);
    snprintf(text_buf, sizeof(text_buf), "%02d", edit_minute_);
    display.get_text_bounds(text_buf, 158, ROW_TIME_Y, &bx, &by, &bw, &bh);
    if (is_active) display.draw_highlight(bx, by, bw, bh);
    display.draw_text(158, ROW_TIME_Y, text_buf, 1, is_active);

    // Footer
    display.draw_line(0, FOOTER_LINE_Y, display.width(), FOOTER_LINE_Y);
    display.draw_bitmap(5, FOOTER_ICON_Y, Icons::up_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(28, FOOTER_TEXT_Y, "/");
    display.draw_bitmap(35, FOOTER_ICON_Y, Icons::down_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_text(55, FOOTER_TEXT_Y, ": Edit");
    display.draw_text(166, FOOTER_TEXT_Y, "OK: Next/Save");
}