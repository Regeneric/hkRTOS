add_library(libdht11 
    STATIC
        dht11.c
        dht11_pio.c
        dht11_dma.c
        dht11.pio
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


pico_generate_pio_header(libdht11
    ${CMAKE_CURRENT_LIST_DIR}/dht11.pio
    OUTPUT_DIR ${PROJECT_INC_DIR}/sensors/dht11
)

target_include_directories(libdht11
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libdht11
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
)