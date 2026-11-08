#include "SGP.hpp"
#include "sensirion.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "SGP";

// SGP41 commands
// 4.7 I2C commands, page 13..17 from: https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf
static constexpr uint16_t CMD_EXECUTE_CONDITIONING = 0x2612;
static constexpr uint16_t CMD_MEASURE_RAW_SIGNALS = 0x2619;
static constexpr uint16_t CMD_TURN_HEATER_OFF = 0x3615;

// SGP41 timings
// 4.7 I2C commands, table 8, page 13 from: https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf
// max 50 ms meassurement duration
static constexpr int MEASURE_DELAY_MS = 50;
// conditioning is recommended for 10s, but no more to avoid damage to the sensor
static constexpr int64_t CONDITIONING_US = 10 * 1000 * 1000;

SGP::SGP()
{
    this->i2cAddress = 0x59;
    this->displayName = "SGP41";
}

SGP::~SGP()
{
    if (device) {
        turnHeaterOff();
        i2c_master_bus_rm_device(device);
        device = nullptr;
    }
}

esp_err_t SGP::init(i2c_master_bus_handle_t bus)
{
    if (device) {
        return ESP_OK;
    }
    esp_err_t err = sensirion::attachDevice(bus, (uint16_t)i2cAddress, displayName.c_str(), &device);
    if (err == ESP_OK) {
        sensorStart = esp_timer_get_time();
    }
    return err;
}

bool SGP::isConditioning() const
{
    return device && (esp_timer_get_time() - sensorStart) < CONDITIONING_US;
}

esp_err_t SGP::measure(float temperatureC, float humidityRH)
{
    sgpValid = false;
    if (!device) {
        return ESP_ERR_INVALID_STATE;
    }

    if (humidityRH < 0.0f) humidityRH = 0.0f;
    if (humidityRH > 100.0f) humidityRH = 100.0f;
    if (temperatureC < -45.0f) temperatureC = -45.0f;
    if (temperatureC > 130.0f) temperatureC = 130.0f;

    const uint16_t rhTicks = (uint16_t)(humidityRH * 65535.0f / 100.0f);
    const uint16_t tTicks = (uint16_t)((temperatureC + 45.0f) * 65535.0f / 175.0f);

    const bool conditioning = isConditioning();
    const uint16_t command = conditioning ? CMD_EXECUTE_CONDITIONING : CMD_MEASURE_RAW_SIGNALS;

    // transmitted package construction
    uint8_t tx[8];
        // subcommand "with humidity compensation"
        tx[0] = (uint8_t)(command >> 8);
        tx[1] = (uint8_t)(command & 0xFF);

        // sending relative humidity in sensor-readable format
        tx[2] = (uint8_t)(rhTicks >> 8);
        tx[3] = (uint8_t)(rhTicks & 0xFF);
        tx[4] = sensirion::crc8(&tx[2]);
        //ends in crc

        // sending temperature in sensor-readable format
        tx[5] = (uint8_t)(tTicks >> 8);
        tx[6] = (uint8_t)(tTicks & 0xFF);
        tx[7] = sensirion::crc8(&tx[5]);
        // ends in crc

    esp_err_t err = i2c_master_transmit(device, tx, sizeof(tx), sensirion::I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "%s command failed: %s", displayName.c_str(), esp_err_to_name(err));
        return err;
    }

    // waits for the sensor to complete sampling
    vTaskDelay(pdMS_TO_TICKS(MEASURE_DELAY_MS));

    // while conditioning, returns only VOC (3 bytes)
    // normal measurement returns VOC + NOx (6 bytes)
    uint8_t rx[6];
    const size_t rxLen = conditioning ? 3 : 6;
    err = i2c_master_receive(device, rx, rxLen, sensirion::I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "%s read failed: %s", displayName.c_str(), esp_err_to_name(err));
        return err;
    }

    // checks validity with CRCs, no second word and CRC expected while conditioning
    if (!sensirion::wordValid(&rx[0]) || (!conditioning && !sensirion::wordValid(&rx[3]))) {
        ESP_LOGW(TAG, "%s CRC mismatch", displayName.c_str());
        return ESP_ERR_INVALID_CRC;
    }

    // converting to readable values
    srawVoc = sensirion::toWord(&rx[0]);
    srawNox = conditioning ? 0 : sensirion::toWord(&rx[3]);

    sgpValid = true;
    return ESP_OK;
}

esp_err_t SGP::turnHeaterOff()
{
    if (!device) {
        return ESP_ERR_INVALID_STATE;
    }
    const uint8_t cmd[2] = {
        (uint8_t)(CMD_TURN_HEATER_OFF >> 8),
        (uint8_t)(CMD_TURN_HEATER_OFF & 0xFF),
    };
    return i2c_master_transmit(device, cmd, sizeof(cmd), sensirion::I2C_TIMEOUT_MS);
}
