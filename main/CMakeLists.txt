file(GLOB UI_SOURCES "../RuneGUI/src/ui/*.c")
file(GLOB_RECURSE MAIN_SOURCES CMAKE_CONFIGURE_DEPENDS 
"*.cpp")

idf_component_register(SRCS ${UI_SOURCES} "${MAIN_SOURCES}"
                    INCLUDE_DIRS "." "ui/" "../RuneGUI/src/ui/" "snake/"
                    REQUIRES "lvgl"
                            bsp
                            espressif__esp_lcd_touch
                            espressif__esp_lcd_touch_gt911
                    )
