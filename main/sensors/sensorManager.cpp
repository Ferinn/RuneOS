#include "sensorManager.hpp"

#include "esp_log.h"

SensorManager::SensorManager()
{
    this->sht = SHT();
    this->sgp = SGP();
}

esp_err_t SensorManager::init()
{
    if (initialised) {
        return ESP_OK;
    }

    esp_err_t err = bsp_i2c_init();
    if (err != ESP_OK) {
        ESP_LOGE("SensorManager", "bsp_i2c_init failed: %s", esp_err_to_name(err));
        return err;
    }

    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();

    err = sht.init(bus);
    if (err != ESP_OK) {
        return err;
    }
    err = sgp.init(bus);
    if (err != ESP_OK) {
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
