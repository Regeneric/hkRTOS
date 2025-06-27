add_subdirectory(dhcp)
add_subdirectory(dns)

add_library(libnetwork 
    STATIC
        wifi.c
        mqtt.c
        html.c
        ping.c
)

target_compile_definitions(libnetwork
    PRIVATE
        HTTPD_USE_CUSTOM_FSDATA=1
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
        ${CMAKE_CURRENT_SOURCE_DIR} 
)

target_link_libraries(libnetwork
    PRIVATE
        libcore
        libstorage
    #     libdhcp
    #     libdns

    PUBLIC
        pico_stdlib 
        pico_lwip_http
        pico_lwip_mdns
        pico_cyw43_arch_lwip_sys_freertos

        hardware_gpio
        hardware_flash
        hardware_watchdog

        FreeRTOS-Kernel-Heap4
)