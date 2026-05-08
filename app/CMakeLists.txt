cmake_minimum_required(VERSION 3.20.0)

set(DTC_OVERLAY_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/esp32c3_devkitc.overlay
    ${CMAKE_CURRENT_SOURCE_DIR}/app.overlay
)

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED)
project(my_zephyr_app)

target_sources(app PRIVATE
    src/main.cpp
    drivers/sensor/led_sensor/led_sensor.c
)
