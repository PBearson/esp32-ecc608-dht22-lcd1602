set(COMPONENT_SRCS "main.cpp"
    "ili9340"
    "fontx.c"
    "pngle.c"
    "decode_image.c"
    )

set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

spiffs_create_partition_image(storage ../font FLASH_IN_PROJECT)