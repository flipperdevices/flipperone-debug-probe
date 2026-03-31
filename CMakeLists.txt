# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(TARGET_HARDWARE "f150" CACHE STRING "Target hardware")
set(FIRMWARE_TARGET 150 CACHE STRING "Firmware target number")
set(FIRMWARE_ORIGIN "Official" CACHE STRING "Firmware origin")

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
# TODO: custom board with RP2350_B_
set(PICO_BOARD pico2 CACHE STRING "Board type")
set(TINYUSB_OPT_OS OPT_OS_FREERTOS CACHE STRING "TinyUSB OS option")
set(LOG 0 CACHE STRING "TinyUSB debug level (used by tinyusb family.cmake)")

# Pull in Raspberry Pi Pico SDK (must be before project)
include("lib/sdk/pico_sdk_import.cmake")

project(flipperone_debug_probe C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Enable warnings and treat as errors
add_compile_options(-fno-strict-aliasing)
add_compile_options(-Werror -Wall)

# FreeRTOS has some warnings that we need to disable for now
# TODO: disable these only for FreeRTOS files, not the whole project
add_compile_options(-Wno-unused-variable)
add_compile_options(-Wno-unused-but-set-variable)

# clay.c has some warnings also
add_compile_options(-Wno-unknown-pragmas)

# --- Git version info ---
execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT GIT_COMMIT)
    set(GIT_COMMIT "unknown")
endif()

execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT GIT_BRANCH)
    set(GIT_BRANCH "unknown")
endif()

execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} describe --tags --abbrev=0
    OUTPUT_VARIABLE GIT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT GIT_VERSION)
    set(GIT_VERSION "unknown")
endif()

execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} status --porcelain
    OUTPUT_VARIABLE GIT_DIRTY_CHECK OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(GIT_DIRTY_CHECK)
    set(BUILD_DIRTY 1)
else()
    set(BUILD_DIRTY 0)
endif()

execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} remote get-url origin
    OUTPUT_VARIABLE GIT_ORIGIN OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT GIT_ORIGIN)
    set(GIT_ORIGIN "unknown")
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
# --- end git version info ---

# Add executable. Default name is the project name, version 0.1
file(GLOB_RECURSE SOURCES
    "lib/furi/*.c*" 
    "lib/furi/version/*.c*" 
    "lib/furi/core/*.c*"
    "lib/toolbox/*.c*"
    "app/*.c"
    "app/theme/*.c"
    "app/fs/*.c"
    "targets/${TARGET_HARDWARE}/furi_hal/*.c*"
    "targets/${TARGET_HARDWARE}/furi_bsp/*.c*"
    "lib/tusb/*.c*"
    "applications/*.c"
    "applications/services/*.c"
    "applications/main/*.c"
    "targets/${TARGET_HARDWARE}/src/*.c*" 
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

set(VERSION_COMPILE_DEFS
    "GIT_COMMIT=\"${GIT_COMMIT}\""
    "GIT_BRANCH=\"${GIT_BRANCH}\""
    "VERSION=\"${GIT_VERSION}\""
    "BUILD_DIRTY=${BUILD_DIRTY}"
    "GIT_ORIGIN=\"${GIT_ORIGIN}\""
    "BUILD_DATE=\"${BUILD_DATE}\""
    "TARGET=${FIRMWARE_TARGET}"
    "FIRMWARE_ORIGIN=\"${FIRMWARE_ORIGIN}\""
)
set_source_files_properties(lib/furi/version/version.c PROPERTIES
    COMPILE_DEFINITIONS "${VERSION_COMPILE_DEFS}")

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    PICO_DEFAULT_UART_TX_PIN=0
    PICO_DEFAULT_UART_RX_PIN=1
    PICO_DEFAULT_UART_BAUD_RATE=230400
    PARAM_ASSERTIONS_ENABLE_ALL=1
    USE_PRINTF
    USE_DBG_PRINTF
    PICO_USE_MALLOC_MUTEX=0
    # https://www.raspberrypi.com/documentation/pico-sdk/high_level.html 
    # Set PICO_FLASH_ASSUME_CORE1_SAFE=1 to explicitly say that core 1 is never using flash. 
    # TODO: When enabling multi-core, this needs to be disabled and the correct flash access mechanism implemented
    PICO_FLASH_ASSUME_CORE1_SAFE=1
    #PICO_USE_SW_SPIN_LOCKS=0
)


pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 0)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/applications
    ${CMAKE_CURRENT_LIST_DIR}/applications/services
    ${CMAKE_CURRENT_LIST_DIR}/lib/toolbox
    ${CMAKE_CURRENT_LIST_DIR}/lib/furi
    ${CMAKE_CURRENT_LIST_DIR}/lib/tusb
    ${CMAKE_CURRENT_LIST_DIR}/targets/${TARGET_HARDWARE}/src
    ${CMAKE_CURRENT_LIST_DIR}/targets/${TARGET_HARDWARE}/furi_hal
    ${CMAKE_CURRENT_LIST_DIR}/targets/${TARGET_HARDWARE}/furi_bsp
    ${CMAKE_CURRENT_LIST_DIR}/targets/${TARGET_HARDWARE}/config
    ${CMAKE_CURRENT_LIST_DIR}/lib
    ${CMAKE_CURRENT_LIST_DIR}/lib/freertos
    ${CMAKE_CURRENT_LIST_DIR}/lib/mlib
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/uart_pio)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/mlib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/freertos)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pico-kvstore)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/debug_probe)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
    cmsis_core
    pico_stdlib
    hardware_pwm
    hardware_adc
    hardware_dma
    hardware_uart
    hardware_watchdog
    hardware_clocks
    pico_unique_id
    uart_pio
    tinyusb_device
    freertos
    mlib
    kvstore
    kvstore_default
    debug_probe
)

# executing code from RAM
pico_set_binary_type(${PROJECT_NAME} copy_to_ram)
#pico_set_binary_type(${PROJECT_NAME} no_flash)

pico_add_extra_outputs(${PROJECT_NAME})
