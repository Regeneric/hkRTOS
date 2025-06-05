cmake_minimum_required(VERSION 3.17)

# --- BUILD TYPE --------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()


option(BUILD_FOR_ARM   "Build for ARM platform"    OFF)     # RP2040/RP2350 ARM build target
option(BUILD_FOR_RISCV "Build for RISC-V platform" OFF)     # RP2350 RISC-V build target

if(BUILD_FOR_ARM)
    add_compile_definitions(HPLATFORM_ARM)
    add_compile_definitions(__arm_gcc__)

    if(NOT PICO_BOARD)
        set(PICO_BOARD "pico2")
        message("PICO_BOARD HASN'T BEEN DECLARED. SETTING IT TO pico2")
    endif()

    if(NOT PICO_PLATFORM)
        set(PICO_PLATFORM "rp2350-arm-s")
        message("PICO_PLATFORM HASN'T BEEN DECLARED. SETTING IT TO rp2350-arm-s")
    endif()
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(HPLATFORM_RISCV)
    add_compile_definitions(__riscv_gcc__)

    if(NOT PICO_BOARD)
        set(PICO_BOARD "pico2")
        message("PICO_BOARD HASN'T BEEN DECLARED. SETTING IT TO pico2")
    endif()

    if(NOT PICO_PLATFORM)
        set(PICO_PLATFORM "rp2350-riscv")
        message("PICO_PLATFORM HASN'T BEEN DECLARED. SETTING IT TO rp2350-riscv")
    endif()
else()
    message(FATAL_ERROR "Compile platform has not been specified.")
endif()


# --- RTOS KERNEL & SDK PATHS -------------------------------------------------
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel)
set(PICO_SDK ${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-sdk)
set(PICO_EXTRAS ${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-extras)
set(PICO_DEOPTIMIZED_DEBUG 0)

include(pico_sdk_import.cmake)


if(BUILD_FOR_ARM)
    if(PICO_BOARD STREQUAL "pico" OR PICO_PLATFORM STREQUAL "rp2040")
        include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
    elseif(PICO_BOARD STREQUAL "pico2" OR PICO_PLATFORM STREQUAL "rp2350-arm-s")
        add_compile_definitions(PICO_RP2350)
        include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)
    else()
        message(FATAL_ERROR "Pico platform has not been specified.")
    endif()
elseif(BUILD_FOR_RISCV)
    add_compile_definitions(PICO_RP2350)
    include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_RISC-V/FreeRTOS_Kernel_import.cmake)
else()
    message(FATAL_ERROR "Compile platform has not been specified.")
endif()



project(hkRTOS)

pico_sdk_init()

message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")


# Use C11 everywhere
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use C++17 everywhere
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json for VSCode/IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_INC_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(
    ${PROJECT_INC_DIR}
    ${PROJECT_INC_DIR}/config
    ${CMAKE_SOURCE_DIR}/lib/SEGGER
    ${CMAKE_SOURCE_DIR}/lib/SEGGER/Config
    ${CMAKE_SOURCE_DIR}/lib/SEGGER/FreeRTOSV11
)


add_compile_options(
    -Wno-format                   # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
)

# if you have anything in "lib" folder then uncomment below - remember to add a CMakeLists.txt
# file to the "lib" directory
add_subdirectory(lib/SEGGER)

# Add subdirectories
add_subdirectory(src)


if(CMAKE_EXPORT_COMPILE_COMMANDS)
  file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/.vscode")

  add_custom_target(
    copy_compile_commands ALL
    COMMENT "⋯ copying compile_commands.json into .vscode/ ⋯"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/.vscode/compile_commands.json"
  )
endif()