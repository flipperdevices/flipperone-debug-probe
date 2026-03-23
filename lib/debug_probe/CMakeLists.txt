project("debug_probe")

add_library(${PROJECT_NAME} INTERFACE)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/probe.pio)

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Include/
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/Core/Include/
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Collect sources from src/ (non-absolute path) and CMSIS DAP sources.
file(GLOB_RECURSE DEBUG_PROBE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
)

set(CMSIS_DAP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP.c
    ${CMAKE_CURRENT_LIST_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/JTAG_DP.c
    ${CMAKE_CURRENT_LIST_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP_vendor.c
    ${CMAKE_CURRENT_LIST_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/SWO.c
    #${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/SW_DP.c
)

set(OTHER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/hid_glue.c
)

target_sources(${PROJECT_NAME} INTERFACE
    ${DEBUG_PROBE_SOURCES}
    ${CMSIS_DAP_SOURCES}
    ${OTHER_SOURCES}
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    hardware_pio
)

target_include_directories(${PROJECT_NAME} INTERFACE ${INCLUDES})