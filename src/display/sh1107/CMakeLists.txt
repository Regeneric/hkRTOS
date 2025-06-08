add_library(libsh1107 
    STATIC
        sh1107.c
        sh1107_gfx.c
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


target_include_directories(libsh1107
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libsh1107
    PUBLIC
        pico_stdlib 

        hardware_gpio
        hardware_i2c 
        hardware_spi
)