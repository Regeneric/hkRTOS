add_library(libonewire 
    STATIC
        onewire.c
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


pico_generate_pio_header(libonewire
    ${CMAKE_CURRENT_LIST_DIR}/onewire_reset.pio
    OUTPUT_DIR ${PROJECT_INC_DIR}/comm/onewire
)

pico_generate_pio_header(libonewire
    ${CMAKE_CURRENT_LIST_DIR}/onewire_write.pio
    OUTPUT_DIR ${PROJECT_INC_DIR}/comm/onewire
)

pico_generate_pio_header(libonewire
    ${CMAKE_CURRENT_LIST_DIR}/onewire_read.pio
    OUTPUT_DIR ${PROJECT_INC_DIR}/comm/onewire
)


target_include_directories(libonewire
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libonewire
    PUBLIC
        pico_stdlib 
        pico_cyw43_arch_lwip_sys_freertos

        hardware_gpio
        hardware_pio
        hardware_i2c 

        FreeRTOS-Kernel
)