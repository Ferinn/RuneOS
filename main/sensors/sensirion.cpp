#include "sensirion.hpp"

#include <cstddef>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void sensirion::waitAtLeastMs(int ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms) + 1);
}

uint8_t sensirion::crc8(const uint8_t data[2])
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < 2; i++)
    {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1) ^ 0x31u;
            }
            else
            {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

bool sensirion::wordValid(const uint8_t* words3)
{
    return crc8(words3) == words3[2];
}

uint16_t sensirion::toWord(const uint8_t* bytes)
{
    return (uint16_t)((bytes[0] << 8) | bytes[1]);
}

esp_err_t sensirion::attachDevice(i2c_master_bus_handle_t bus, uint16_t address,
                                const char* name, i2c_master_dev_handle_t* outDevice)
{
    esp_err_t err = i2c_master_probe(bus, address, I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGE("Sensirion", "%s not found at 0x%02X: %s", name, address, esp_err_to_name(err));
        return err;
    }

    i2c_device_config_t cfg = {};
    cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    cfg.device_address = address;
    cfg.scl_speed_hz = CONFIG_BSP_I2C_CLK_SPEED_HZ;

    err = i2c_master_bus_add_device(bus, &cfg, outDevice);
    if (err != ESP_OK) {
        ESP_LOGE("Sensirion", "Failed to add %s: %s", name, esp_err_to_name(err));
        return err;
    }

    ESP_LOGI("Sensirion", "%s found at 0x%02X", name, address);
    return ESP_OK;
}