add_library(libsensors 
    STATIC
        dht11.c
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


pico_generate_pio_header(libsensors
    ${CMAKE_CURRENT_LIST_DIR}/dht11.pio
    OUTPUT_DIR ${PROJECT_INC_DIR}/sensors
)

target_include_directories(libsensors
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libsensors
    PRIVATE
        libcore

    PUBLIC
        pico_stdlib 

        hardware_gpio
        hardware_i2c
        hardware_pio
        hardware_clocks

        FreeRTOS-Kernel
)