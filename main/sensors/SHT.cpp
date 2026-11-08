#include "SHT.hpp"
#include "sensirion.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "SHT";

// SHT40 command(s)
// 4.5 Command overview, page 12/25 from: https://sensirion.com/media/documents/33FD6951/6A7C10A0/HT_DS_Datasheet_SHT4x_V7.3.pdf
// Using only high precision meassurement
static constexpr uint8_t CMD_MEASURE_HIGH_PRECISION = 0xFD;

// SHT40 timing(s)
// 3.1 Timings, page 10/25 from: https://sensirion.com/media/documents/33FD6951/6A7C10A0/HT_DS_Datasheet_SHT4x_V7.3.pdf
// sensirion says max is 8.3 ms, this includes slight buffer
static constexpr int MEASURE_DELAY_MS = 10;

SHT::SHT()
{
    i2cAddress = 0x44;
    displayName = "SHT40";
}

SHT::~SHT()
{
    if (device) {
        i2c_master_bus_rm_device(device);
        device = nullptr;
    }
}

esp_err_t SHT::init(i2c_master_bus_handle_t bus)
{
    if (device) {
        return ESP_OK;
    }
    return sensirion::attachDevice(bus, (uint16_t)i2cAddress, displayName.c_str(), &device);
}

esp_err_t SHT::measure()
{
    shtValid = false;
    if (!device)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // single command byte; SHT40 commands carry no parameters and no CRC
    const uint8_t cmd = CMD_MEASURE_HIGH_PRECISION;
    esp_err_t err = i2c_master_transmit(device, &cmd, 1, sensirion::I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "%s command failed: %s", displayName.c_str(), esp_err_to_name(err));
        return err;
    }

    // waits for the sensor to complete sampling
    sensirion::waitAtLeastMs(MEASURE_DELAY_MS);

    // expects 6 bytes:
        // 2 byte word for temperature
        // 1 CRC
        // 2 byte word for relative humidity
        // 1 CRC
    uint8_t rx[6];
    err = i2c_master_receive(device, rx, sizeof(rx), sensirion::I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "%s read failed: %s", displayName.c_str(), esp_err_to_name(err));
        return err;
    }
    
    // checks validity with CRCs
    if (!sensirion::wordValid(&rx[0]) || !sensirion::wordValid(&rx[3])) {
        ESP_LOGW(TAG, "%s CRC mismatch", displayName.c_str());
        return ESP_ERR_INVALID_CRC;
    }

    // converting to readable values
    const uint16_t tTicks  = sensirion::toWord(&rx[0]);
    const uint16_t rhTicks = sensirion::toWord(&rx[3]);

    // transforms raw data into final values based on SENSIRIONS docs
    // 4.6 Conversion of Signal Ouput, page 12/25 from: ofhttps://sensirion.com/media/documents/33FD6951/6A7C10A0/HT_DS_Datasheet_SHT4x_V7.3.pdf
    temperatureC = -45.0f + 175.0f * (float)tTicks / 65535.0f;
    humidityRH   =  -6.0f + 125.0f * (float)rhTicks / 65535.0f;
    if (humidityRH < 0.0f)   humidityRH = 0.0f;
    if (humidityRH > 100.0f) humidityRH = 100.0f;

    shtValid = true;
    return ESP_OK;
}
