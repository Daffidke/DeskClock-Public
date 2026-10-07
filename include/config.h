#pragma once
#include <stdint.h>

namespace Pins {
    constexpr uint8_t BATTERY_ADC   = 0;

    constexpr uint8_t BTN_UP        = 1;
    constexpr uint8_t BTN_SELECT    = 2;
    constexpr uint8_t BTN_DOWN      = 3;

    constexpr uint8_t CHARGE_DETECT = 4;

    constexpr uint8_t I2C_SDA       = 19;
    constexpr uint8_t I2C_SCL       = 5;
    
    constexpr uint8_t BUZZER        = 6;
    constexpr uint8_t LED_DATA      = 8;

    constexpr uint8_t SPI_SDA       = 7;
    constexpr uint8_t SPI_RES       = 9;
    constexpr uint8_t SPI_SCL       = 14;
    constexpr uint8_t SPI_CS        = 15;
    constexpr uint8_t SPI_DC        = 18;
    constexpr uint8_t SPI_BUSY      = 20;
}

namespace TimingConfig {
    constexpr uint32_t BAUD_RATE            = 115200;
    constexpr uint32_t I2C_FREQ             = 100000;
    constexpr uint32_t BTN_DEBOUNCE_MS      = 50;
    constexpr uint32_t BTN_LONG_PRESS_MS    = 600;
    constexpr uint8_t  FULL_REFRESH_ON_MIN  = 10;
}