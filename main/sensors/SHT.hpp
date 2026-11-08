#pragma once

#include <cstdint>
#include <string>
#include "driver/i2c_master.h"

// SHT40 temp & humidity sensor
class SHT
{
    public:
        // uses the hardcoded i2c address: 0x44
        SHT();
        // only frees resources, heating element is unused as of now
        ~SHT();

        // non-copyable: a copy would share the I2C device handle and its destructor
        // would remove the device from the bus while the original still uses it
        SHT(const SHT&) = delete;
        SHT& operator=(const SHT&) = delete;

        // attaches the sensor device and stores it in this instance
        esp_err_t init(i2c_master_bus_handle_t bus);

        // recieves and stores data into private temperatureC, humidityRH
        // hardcoded to use the high precision (high repeatability) meassurement command
        // 4.6 Conversion of Signal Ouput, page 12/25 from: ofhttps://sensirion.com/media/documents/33FD6951/6A7C10A0/HT_DS_Datasheet_SHT4x_V7.3.pdf
        // for raw data interpretation
        esp_err_t measure();

        //readonly getters
        float getTemperatureC() const { return temperatureC; }
        float getHumidityRH() const { return humidityRH; }
        bool isValid() const { return shtValid; }
        bool isAttached() const { return device != nullptr; }
        int getAddress() const { return i2cAddress; }
        const std::string& getName() const { return displayName; }

    private:
        int i2cAddress;
        std::string displayName;
        i2c_master_dev_handle_t device = nullptr;
        float temperatureC = -273.15f;
        float humidityRH = 0.0f;
        bool shtValid = false;
};
