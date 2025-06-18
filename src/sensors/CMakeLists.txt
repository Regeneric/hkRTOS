add_library(libsensors INTERFACE)

add_subdirectory(dht11_22)
add_subdirectory(dht20)
add_subdirectory(ds18b20)
add_subdirectory(bme280)
add_subdirectory(pms5003)
add_subdirectory(sgp30)

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
        libdht11_22
        libdht20
        libds18b20
        libpms5003
        libsgp30
        libbme280
)