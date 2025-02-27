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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(projeto_caldeira C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(projeto_caldeira projeto_caldeira.c lib/ssd1306.c)

pico_set_program_name(projeto_caldeira "projeto_caldeira")
pico_set_program_version(projeto_caldeira "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(projeto_caldeira 1)
pico_enable_stdio_usb(projeto_caldeira 1)

pico_generate_pio_header(projeto_caldeira ${CMAKE_CURRENT_LIST_DIR}/pio_matrix.pio)

target_sources(projeto_caldeira PRIVATE projeto_caldeira.c)

# Add the standard library to the build
target_link_libraries(projeto_caldeira PRIVATE
        pico_stdlib
        hardware_pio
        hardware_adc
        pico_bootrom
        hardware_i2c
        hardware_pwm
        )

# Add the standard include files to the build
target_include_directories(projeto_caldeira PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(projeto_caldeira 
        
        )

pico_add_extra_outputs(projeto_caldeira)

