#pragma once

#include <cstdio>
#include <cstdint>
#include <string>
#include "bsp/esp-bsp.h"

#include "SHT.hpp"
#include "SGP.hpp"

class SensorManager
{
    public:
        // creates SHT and SGP
        SensorManager();

        // Initialises the BSP I2C bus and attaches both sensors
        esp_err_t init();

        // Measures SHT40, then calls SGP41 to meassure adjusted by the SHT40 values
        // Expects sampling at 1Hz frequency
        esp_err_t update();

        // readonly getters
        const SHT& getSHT() const { return sht; };
        const SGP& getSGP() const { return sgp; };

    private:
        SHT sht;
        SGP sgp;
        bool initialised = false;
};