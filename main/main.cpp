#include <stdio.h>
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "ui.h"
#include "screens.h"
#include "actions.h"

#include "ui/graphManager.hpp"
#include "snake/snake.hpp"

extern "C" void app_main()
{
    printf("RuneOS initialising screen...\n");
    lv_display_t* display = bsp_display_start();
    bsp_display_backlight_on();

    bsp_display_lock(portMAX_DELAY);

    // EEZ Studio UI init
    ui_init();
    // RuneOS UI init
    printf("TEST!!!");
    
    //GraphManager* gm = new GraphManager(SENSOR);

    bsp_display_unlock();
    printf("RuneOS screen initialised!\n");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

