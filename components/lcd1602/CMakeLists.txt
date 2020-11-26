cmake_minimum_required(VERSION 3.5)

set(COMPONENT_ADD_INCLUDEDIRS include)

set(COMPONENT_SRCS "src/DHT22.cpp"
        "src/esp32-hal-gpio.c"
        "src/esp32-hal-misc.c"
        "src/IPAddress.cpp"
        "src/LiquidCrystal.cpp"
        "src/Print.cpp")

set(COMPONENT_REQUIRES nvs_flash spi_flash)

register_component()