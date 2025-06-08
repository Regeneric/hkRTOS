add_library(libnetwork 
    STATIC
        wifi.c
        # http_server.c
        # http_client.c
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


target_include_directories(libnetwork
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libnetwork
    PUBLIC
        pico_stdlib 
        # pico_cyw43_arch_lwip_sys_freertos
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http

        hardware_gpio
        hardware_pio
        hardware_i2c 

        FreeRTOS-Kernel
)