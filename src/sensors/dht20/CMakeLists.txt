add_library(libdht20 
    STATIC
        dht20.c
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


target_include_directories(libdht20
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libdht20
    PRIVATE
        libcore

    PUBLIC
        pico_stdlib 

        hardware_gpio
        hardware_pio
        hardware_clocks
        hardware_dma
        hardware_irq

        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4 
)