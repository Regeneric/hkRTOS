add_library(libsgp30
    STATIC
        sgp30.c
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


target_include_directories(libsgp30
    PUBLIC
        ${PROJECT_INC_DIR}
)

target_link_libraries(libsgp30
    PRIVATE
        libcore
        libcomm

    PUBLIC
        pico_stdlib 

        hardware_gpio
        hardware_uart

        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
)