#pragma once

#include <cstdint>
#include <string>
#include "driver/i2c_master.h"

// SGP41 VOC & NOx
class SGP
{
    public:
        // uses the hardcoded i2c address: 0x59
        SGP();
        // outside of freeing resources, this also turns the sensor's heater off
        ~SGP();

        // attaches the sensor device and stores it in this instance
        // starts a timer for MEASURE_DELAY_MS entering conditioning phase for roughly that time
        esp_err_t init(i2c_master_bus_handle_t bus);
        
        // expects to be called roughly 1Hz
        // uses table 12, page 15 from: https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf
        // for raw data interpretation
        // transmits data based on the transformation shown in the material, recieves data into private srawVoc and srawNox
        esp_err_t measure(float temperatureC = 25.0f, float humidityRH = 50.0f);

        // turns the sensor's heater off and returns it into idle state
        esp_err_t turnHeaterOff();

        // getters
        uint16_t getSrawVoc() const { return srawVoc; }
        uint16_t getSrawNox() const { return srawNox; }
        bool isConditioning() const;
        bool isValid() const { return sgpValid; }
        bool isAttached() const { return device != nullptr; }
        uint8_t getAddress() const { return i2cAddress; }
        const std::string& getName() const { return displayName; }

    private:
        int i2cAddress;
        std::string displayName;
        i2c_master_dev_handle_t device = nullptr;
        int64_t sensorStart = 0;
        uint16_t srawVoc = 0;
        uint16_t srawNox = 0;
        bool sgpValid = false;
};
