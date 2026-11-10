file(GLOB_RECURSE MAIN_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB UI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../RuneGUI/src/ui/*.c")

idf_component_register(
    SRCS ${MAIN_SOURCES} ${UI_SOURCES}
    INCLUDE_DIRS "." "../RuneGUI/src/ui/"
    REQUIRES lvgl
             bsp
             esp_driver_i2c
             esp_timer
)