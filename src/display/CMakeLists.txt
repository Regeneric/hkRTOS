add_library(libdisplay INTERFACE)

add_subdirectory(gfx)
add_subdirectory(ssd1327)
add_subdirectory(sh1107)


if(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)
else()
    message(WARNING "Unknown platform — no compile options set")
endif()


target_link_libraries(libdisplay
    INTERFACE
        # libsh1107
        libssd1327
        libgfx
        # libbme280
)