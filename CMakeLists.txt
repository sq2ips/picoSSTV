# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picoSSTV C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

include_directories(include)
add_executable(picoSSTV 
    src/main.c
    src/sstv.c
    include/sstv.h
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/image_bin.o
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_LIST_DIR}/image.bin
            ${CMAKE_CURRENT_BINARY_DIR}/image.bin
    COMMAND ${CMAKE_OBJCOPY}
            -I binary
            -O elf32-littlearm
            -B arm
            image.bin
            image_bin.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/image.bin
)

add_custom_target(image_bin_target
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/image_bin.o
)

add_dependencies(picoSSTV image_bin_target)

target_sources(picoSSTV PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/image_bin.o
)

pico_set_program_name(picoSSTV "picoSSTV")
pico_set_program_version(picoSSTV "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(picoSSTV 0)
pico_enable_stdio_usb(picoSSTV 1)

# Add the standard library to the build
target_link_libraries(picoSSTV
        pico_stdlib hardware_pwm pico_multicore)

# Add the standard include files to the build
target_include_directories(picoSSTV PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(picoSSTV)

