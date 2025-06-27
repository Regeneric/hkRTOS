add_subdirectory(core)
add_subdirectory(sensors)
add_subdirectory(comm)
add_subdirectory(storage)
add_subdirectory(display)
add_subdirectory(input)
add_subdirectory(timing)

add_executable(${PROJECT_NAME}
    main.c
    collect.c
    process.c
    display.c
    manager.c
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        libcore
        libsensors
        libcomm
        libstorage
        libdisplay
        libinput
        libtiming

        segger_sysview
        segger_sysview_freertos
        segger_rtt

    PUBLIC
        pico_stdlib 
        pico_stdio
        pico_multicore
        pico_sync
        # pico_lwip
        pico_cyw43_arch_lwip_sys_freertos

        hardware_pio
        # hardware_gpio
        # hardware_irq
        # hardware_timer
        # hardware_i2c

        FreeRTOS-Kernel-Heap4
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})


if(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)

    if(PICO_BOARD STREQUAL "pico2" OR PICO_PLATFORM STREQUAL "rp2350-arm-s")
        add_compile_definitions(PICO_RP2350)
    endif()

    target_compile_options(${PROJECT_NAME} 
        PRIVATE
            $<$<CONFIG:Debug>:-O0 -g -std=gnu11>
            $<$<CONFIG:Release>:-Os -std=gnu11>
    )
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(PICO_RP2350)
    add_compile_definitions(__riscv_gcc__)
    
    target_compile_options(${PROJECT_NAME} 
        PRIVATE
            $<$<CONFIG:Debug>:-O0 -g -std=gnu11>
            $<$<CONFIG:Release>:-Os -std=gnu11>
    )
else()
    message(WARNING "Unknown platform — no compile options set")
endif()