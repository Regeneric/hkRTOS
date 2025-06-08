add_library(libssd1327 
    STATIC
        ssd1327.c
        ssd1327_gfx.c
)


if(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)
else()
    message(WARNING "Unknown platform — no compile options set")
endif()


target_include_directories(libssd1327
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libssd1327
    PUBLIC
        pico_stdlib 

        hardware_gpio
        hardware_i2c 
        hardware_spi
)