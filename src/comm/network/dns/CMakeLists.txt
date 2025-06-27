add_library(libdns
    STATIC
        dnsserver.c
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


target_include_directories(libdns
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libdns
    PUBLIC
        pico_stdlib 
        pico_cyw43_arch_lwip_sys_freertos

        FreeRTOS-Kernel-Heap4
)