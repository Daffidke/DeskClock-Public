#include "ui/screens/screen_menu.h"

#include "display_manager.h"
#include "app_state.h"
#include "ui/icons.h"

namespace {
    constexpr int16_t HEADER_LINE_Y = 24;
    constexpr int16_t ROW_START_Y   = 50;
    constexpr int16_t ROW_STRIDE_Y  = 26;
    constexpr int16_t ITEM_TEXT_X   = 16;

    constexpr const char* const MENU_ITEMS[] = {
        "Set Date and Time",
        "Set Alarm",
        "Set Timer",
        "Wi-Fi Setup",
        "Factory Reset",
        "Exit"
    };

    static constexpr uint8_t MENU_ITEM_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);
    static constexpr uint8_t VISIBLE_ITEMS   = 3;
}

void ScreenMenu::on_enter() {
    selected_index_ = 0;
    scroll_offset_  = 0;
}

InputResult ScreenMenu::handle_input(ButtonId btn, ButtonEvent evt, AppState&) {
    InputResult result;

    if (evt == ButtonEvent::LONG_PRESS && btn == ButtonId::SELECT) {
        result.target_screen = ScreenId::MAIN;
        result.refresh = RefreshType::FULL;
        return result;
    }

    if (evt != ButtonEvent::SHORT_PRESS) return result;
    
    result.refresh = RefreshType::PARTIAL;
    result.dirty_rect = DIRTY_AREA;

    switch (btn) {
        case ButtonId::UP:
            if (selected_index_ == 0) {
                selected_index_ = MENU_ITEM_COUNT - 1;
                scroll_offset_ = MENU_ITEM_COUNT - VISIBLE_ITEMS;
            } else {
                selected_index_--;
                if (selected_index_ < scroll_offset_) {
                    scroll_offset_ = selected_index_;
                }
            }
            break;

        case ButtonId::DOWN:
            selected_index_ = (selected_index_ + 1) % MENU_ITEM_COUNT;
            if (selected_index_ == 0) {
                scroll_offset_ = 0;
            } else if (selected_index_ >= scroll_offset_ + VISIBLE_ITEMS){            
                scroll_offset_ = selected_index_ - VISIBLE_ITEMS + 1;
            }
            break;

        case ButtonId::SELECT:
            result.refresh = RefreshType::FULL;
            switch (selected_index_) {
                case 0:
                    result.target_screen = ScreenId::SET_TIME;
                    break;
                case 1:
                    result.target_screen = ScreenId::SET_ALARM;
                    break;
                case 2:
                    result.target_screen = ScreenId::SET_TIMER;
                    break;
                case 3:
                    result.target_screen = ScreenId::SET_WIFI;
                    break;
                case 4:
                    result.target_screen = ScreenId::FACTORY_RESET;
                    break;
                case 5:
                    result.target_screen = ScreenId::MAIN;
                    break;
                default: break;
            }
            break;
        default:
            result.refresh = RefreshType::NONE;    
            break;
    }
    return result;
}

void ScreenMenu::render(const AppState&, DisplayManager& display) {
    int16_t bx, by;
    uint16_t bw, bh;
    
    // Header
    display.draw_text(5, 18, "Main Menu");
    display.draw_line(0, HEADER_LINE_Y, display.width(), HEADER_LINE_Y);

    // Scroll Indicators
    display.draw_bitmap(display.width() - 30, 30, Icons::up_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);
    display.draw_bitmap(display.width() - 30, display.height() - 40, Icons::down_arrow, Icons::ICON_WIDTH, Icons::ICON_HEIGHT);

    // Body
    for (uint8_t i = 0; i < VISIBLE_ITEMS; i++) {
        const uint8_t item_index = i + scroll_offset_;
        if (item_index >= MENU_ITEM_COUNT) break;

        const int16_t text_y = ROW_START_Y + (i * ROW_STRIDE_Y);
        bool is_selected = (item_index == selected_index_);

        if (is_selected) {
            display.get_text_bounds(MENU_ITEMS[item_index], ITEM_TEXT_X, text_y, &bx, &by, &bw, &bh);
            display.draw_highlight(bx, by, bw, bh);
            display.draw_text(ITEM_TEXT_X, text_y, MENU_ITEMS[item_index], 1, true);
        } else {
            display.draw_text(ITEM_TEXT_X, text_y, MENU_ITEMS[item_index], 1, false);
        }
    }
}