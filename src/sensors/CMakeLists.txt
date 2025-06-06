add_library(libsensors INTERFACE)

add_subdirectory(dht11)

if(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)
else()
    message(WARNING "Unknown platform — no compile options set")
endif()

target_link_libraries(libsensors
    INTERFACE
        libdht11
        # libbme280
)