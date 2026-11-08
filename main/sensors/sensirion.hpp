#pragma once

#include <cstdint>
#include "driver/i2c_master.h"
#include "esp_log.h"

// shared sensirion communication helper functions
namespace sensirion
{
    constexpr int I2C_TIMEOUT_MS = 100;

    // Sensirion CRC-8
    // 4.6 checksum calc, page 13/22 from: https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf
    uint8_t crc8(const uint8_t data[2]);

    // Checks a big endian 16-bit word followed by its CRC byte
    bool wordValid(const uint8_t* words3);

    //combines 2 big-endian bytes into a single 16bit value
    uint16_t toWord(const uint8_t* bytes);

    // Probes address on i2c bus and attaches it as a device
    esp_err_t attachDevice(i2c_master_bus_handle_t bus, uint16_t address,
                            const char* name, i2c_master_dev_handle_t* outDevice);
}