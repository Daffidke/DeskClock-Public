#pragma once

#include <cstdint>
#include <Adafruit_SHT31.h>
#include <SensirionI2CSgp41.h>
#include "app_types.h"
#include "app_state.h"

class SensorManager {
public:
    SensorManager() = default;

    boolean init();
    boolean sample(AppState& state, boolean voc_logging = false);

private:
    Adafruit_SHT31 sht30_;
    SensirionI2CSgp41 sgp41_;

    boolean sht30_ok_ = false;
    boolean sgp41_ok_ = false;
};