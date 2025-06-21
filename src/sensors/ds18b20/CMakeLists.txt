add_library(libds18b20
    STATIC
        ds18b20.c
        ds18b20_dma.c
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


target_include_directories(libds18b20
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libds18b20
    PRIVATE
        libcore
        libcomm

    PUBLIC
        pico_stdlib 

        hardware_gpio
        hardware_i2c
        hardware_dma

        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4 
)