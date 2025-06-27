add_library(libdht11_22 
    STATIC
        dht11_22.c
        dht11_22_pio.c
        dht11_22_dma.c
        dht11_22_rtos.c
        dht11_22.pio
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


pico_generate_pio_header(libdht11_22
    ${CMAKE_CURRENT_LIST_DIR}/dht11_22.pio
    OUTPUT_DIR ${PROJECT_INC_DIR}/sensors/dht11_22
)

target_include_directories(libdht11_22
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libdht11_22
    PRIVATE
        libcore

    PUBLIC
        pico_stdlib 

        hardware_gpio
        hardware_pio
        hardware_clocks
        hardware_dma
        hardware_irq

        FreeRTOS-Kernel-Heap4
)