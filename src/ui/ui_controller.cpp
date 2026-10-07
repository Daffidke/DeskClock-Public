#include "ui_controller.h"
#include "ui/icons.h"

UIController::UIController(AppState& state, DisplayManager& display_driver, NetworkService& net) 
    : state_(state), display_(display_driver), screen_set_time_(net), screen_set_wifi_(net) {}

void UIController::init() {
    state_.prev_battery  = state_.battery;
    state_.prev_charging = state_.is_charging;

    change_screen(ScreenId::MAIN);
}

void UIController::change_screen(ScreenId next_screen) {
    state_.active_screen_id = next_screen;

    switch (state_.active_screen_id) {
        case ScreenId::MAIN: screen_main_.on_enter(state_); break;
        case ScreenId::WEATHER: screen_weather_.on_enter(state_); break;
        case ScreenId::MENU: screen_menu_.on_enter(); break;
        case ScreenId::SET_TIME: screen_set_time_.on_enter(state_); break;
        case ScreenId::SET_ALARM: screen_set_alarm_.on_enter(state_); break;
        case ScreenId::SET_TIMER: screen_set_timer_.on_enter(state_); break;
        case ScreenId::SET_WIFI: screen_set_wifi_.on_enter(state_); break;
        case ScreenId::FACTORY_RESET: screen_factory_reset_.on_enter(); break;
        default: break;
    }

    request_refresh(RefreshType::FULL);
}

void UIController::handle_button(ButtonId btn, ButtonEvent evt) {
    InputResult result;

    switch (state_.active_screen_id) {
        case ScreenId::MAIN: result = screen_main_.handle_input(btn, evt, state_); break;
        case ScreenId::WEATHER: result = screen_weather_.handle_input(btn, evt, state_); break;
        case ScreenId::MENU: result = screen_menu_.handle_input(btn, evt, state_); break;
        case ScreenId::SET_TIME: result = screen_set_time_.handle_input(btn, evt, state_); break;
        case ScreenId::SET_ALARM: result = screen_set_alarm_.handle_input(btn, evt, state_); break;
        case ScreenId::SET_TIMER: result = screen_set_timer_.handle_input(btn, evt, state_); break;
        case ScreenId::SET_WIFI: result = screen_set_wifi_.handle_input(btn, evt, state_); break;
        case ScreenId::FACTORY_RESET: result = screen_factory_reset_.handle_input(btn, evt, state_); break;
        default: break;
    }

    if (result.target_screen != ScreenId::NONE && result.target_screen != state_.active_screen_id) {
        change_screen(result.target_screen);
        return;
    }

    request_refresh(result.refresh, result.dirty_rect);
}

void UIController::draw_battery_bmp() {
    const uint8_t* bmp = nullptr;

    if (state_.is_charging) {
        bmp = Icons::battery_charging;
    } else {
        switch (state_.battery) {
            case BatteryState::FOUR_BAR:   bmp = Icons::battery_full; break;
            case BatteryState::THREE_BAR:  bmp = Icons::battery_three_bar; break;
            case BatteryState::TWO_BAR:    bmp = Icons::battery_two_bar; break;
            case BatteryState::ONE_BAR:    bmp = Icons::battery_one_bar; break;
            case BatteryState::DISCHARGED: bmp = Icons::battery_discharged;
            default: break;
        }
    }

    display_.draw_bitmap(255, 2, bmp, Icons::BAT_ICON_WIDTH, Icons::BAT_ICON_HEIGHT);
}

void UIController::render_callback(void* context) {
    auto* self = static_cast<UIController*>(context);
    if (!self) return;

    switch (self->state_.active_screen_id) {
        case ScreenId::MAIN: self->screen_main_.render(self->state_, self->display_); break;
        case ScreenId::WEATHER: self->screen_weather_.render(self->state_, self->display_); break;
        case ScreenId::MENU: self->screen_menu_.render(self->state_, self->display_); break;
        case ScreenId::SET_TIME: self->screen_set_time_.render(self->state_, self->display_); break;
        case ScreenId::SET_ALARM: self->screen_set_alarm_.render(self->state_, self->display_); break;
        case ScreenId::SET_TIMER: self->screen_set_timer_.render(self->state_, self->display_); break;
        case ScreenId::SET_WIFI: self->screen_set_wifi_.render(self->state_, self->display_); break;
        case ScreenId::FACTORY_RESET: self->screen_factory_reset_.render(self->state_, self->display_); break;
        default: break;
    }

    self->draw_battery_bmp();
}

void UIController::request_refresh(RefreshType type, const PartialBounds& bounds) {
    if (type == RefreshType::NONE) return;

    needs_render_ = true;
    if (type == RefreshType::FULL) {
        force_full_ = true;
        dirty_bounds_ = {0, 0, display_.width(), display_.height()};
    } else {
        dirty_bounds_ = bounds;
    }
}

void UIController::update() {
    if (!needs_render_) return;

    display_.render_frame(render_callback, this, !force_full_, dirty_bounds_.x, dirty_bounds_.y, dirty_bounds_.w, dirty_bounds_.h);
    needs_render_ = false;
    force_full_   = false;
}

void UIController::on_tick() {
    RefreshType refresh = RefreshType::NONE;
    PartialBounds bounds{};

    if (state_.battery != state_.prev_battery || state_.is_charging != state_.prev_charging) {
        state_.prev_battery  = state_.battery;
        state_.prev_charging = state_.is_charging;
        request_refresh(RefreshType::PARTIAL, this->DIRTY_AREA);
    }

    switch (state_.active_screen_id) {
        case ScreenId::MAIN:
            refresh = screen_main_.on_tick(state_);
            bounds  = ScreenMain::DIRTY_AREA;
            break;

        case ScreenId::WEATHER:
            refresh = screen_weather_.on_tick(state_);
            bounds  = ScreenWeather::DIRTY_AREA;
            break;

        case ScreenId::SET_TIME:
            refresh = screen_set_time_.on_tick(state_);
            bounds  = ScreenSetTime::DIRTY_AREA;
            break;

        case ScreenId::SET_TIMER:
            refresh = screen_set_timer_.on_tick(state_);
            bounds  = ScreenSetTimer::DIRTY_AREA;
            break;

        case ScreenId::SET_WIFI:
            refresh = screen_set_wifi_.on_tick(state_);
            break;

        default:
            break;
    }

    request_refresh(refresh, bounds);
}