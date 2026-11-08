#include <stdio.h>
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "ui.h"
#include "screens.h"
#include "actions.h"

#include "ui/graphManager.hpp"
#include "sensors/sensorManager.hpp"

extern "C" void app_main()
{
    printf("RuneOS initialising screen...\n");
    lv_display_t* display = bsp_display_start();
    bsp_display_backlight_on();

    bsp_display_lock(portMAX_DELAY);

    // EEZ Studio UI init
    ui_init();
    // RuneOS UI init
    //GraphManager* gm = new GraphManager(SENSOR);

    bsp_display_unlock();
    printf("RuneOS screen initialised!\n");

    SensorManager* sm = new SensorManager();
    sm->init();

    // fixed 1 Hz period regardless of how long update() takes (SGP41 expects 1 s sampling)
    TickType_t lastWake = xTaskGetTickCount();
    while (1)
    {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000));
        sm->update();
        const SHT& shtref = sm->getSHT();
        printf("SHT40 readings:\nTemperature (C): %.3f\nRelative Humidity: %.3f\n", shtref.getTemperatureC(), shtref.getHumidityRH());
        const SGP& sgpref = sm->getSGP();
        printf("SGP41 readings:\nraw VOC: %u\nraw NOx: %u\n", sgpref.getSrawVoc(), sgpref.getSrawNox());
    }
}

