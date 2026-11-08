#include "sensorManager.hpp"

#include "esp_log.h"

// sht and sgp are default-constructed as members before this body runs
SensorManager::SensorManager()
{
}

esp_err_t SensorManager::init()
{
    if (initialised)
    {
        return ESP_OK;
    }

    esp_err_t err = bsp_i2c_init();
    if (err != ESP_OK)
    {
        ESP_LOGE("SensorManager", "bsp_i2c_init failed: %s", esp_err_to_name(err));
        return err;
    }

    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();

    for (uint16_t addr = 0x08; addr < 0x78; addr++)
    {
        esp_err_t r = i2c_master_probe(bus, addr, 50);
        if (r == ESP_OK)
        {
            printf("I2CScan Device found at 0x%02X\n", addr);
        }
        else if (r != ESP_ERR_NOT_FOUND)
        {
            printf("I2CScan 0x%02X: %s\n", addr, esp_err_to_name(r));
        }
    }
    printf("finished scanning!\n");

    err = sht.init(bus);
    if (err != ESP_OK)
    {
        return err;
    }
    err = sgp.init(bus);
    if (err != ESP_OK)
    {
        return err;
    }

    initialised = true;
    ESP_LOGI("SensorManager", "Sensors initialised on I2C%d (SDA=%d, SCL=%d)",
             BSP_I2C_NUM, (int)BSP_I2C_SDA, (int)BSP_I2C_SCL);
    return ESP_OK;
}

esp_err_t SensorManager::update()
{
    if (!initialised) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t shtErr = sht.measure();
    esp_err_t sgpErr = sht.isValid() ? sgp.measure(sht.getTemperatureC(), sht.getHumidityRH())
                                     : sgp.measure();

    return (shtErr != ESP_OK) ? shtErr : sgpErr;
}
