#include "sensor_manager.h"
#include <Wire.h>
#include "config.h"
extern "C" {
    #include <algorithm/sensirion_gas_index_algorithm.h>
}

namespace {
    constexpr uint8_t  SHT30_I2C_ADDR = 0x44;
}

bool SensorManager::init() {
    Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL, TimingConfig::I2C_FREQ);

    sht30_ok_ = sht30_.begin(SHT30_I2C_ADDR);
    
    sgp41_.begin(Wire);
    uint16_t serial_number[3];
    sgp41_ok_ = (sgp41_.getSerialNumber(serial_number) == 0);

    return (sht30_ok_ || sgp41_ok_);
}

bool SensorManager::sample(AppState& state, boolean voc_logging) {
    boolean sht_valid = false;
    float temp = 25.0f;
    float hum = 50.0f;
    
    if (sht30_ok_) {
        temp = sht30_.readTemperature();
        hum = sht30_.readHumidity();

        if (!isnan(temp) && !isnan(hum)) {
            state.sensors.temp_c = temp;
            state.sensors.humidity = hum;
            sht_valid = true;
        }
    }
    state.sensors.valid = sht_valid;

    if (sgp41_ok_) {
        uint16_t comp_rh = static_cast<uint16_t>((hum / 100.0f) * 65535.0f);
        uint16_t comp_t = static_cast<uint16_t>(((temp + 45.0f) / 175.0f) * 65535.0f);

        uint16_t raw_voc = 0;
        uint16_t err;
        for (uint8_t i = 0; i < 3; ++i) {
            err = sgp41_.executeConditioning(comp_rh, comp_t, raw_voc);
        }

        sgp41_.turnHeaterOff();

        if (err == 0 && raw_voc > 0) {
            if (!state.rtc_voc_initialized) {
                GasIndexAlgorithm_init_with_sampling_interval(&state.rtc_voc_params, GasIndexAlgorithm_ALGORITHM_TYPE_VOC, 60.0f);
                state.rtc_voc_initialized = true;
            }

            int32_t voc_index = 0;
            GasIndexAlgorithm_process(&state.rtc_voc_params, (int32_t)raw_voc, &voc_index);
            state.sensors.voc_index = voc_index;
        }
    }

    if (voc_logging) {
        Serial1.begin(115200, SERIAL_8N1, -1, 16);
        Serial1.printf("%d\n", state.sensors.voc_index);
        Serial1.flush();
    }

    return state.sensors.valid;
}