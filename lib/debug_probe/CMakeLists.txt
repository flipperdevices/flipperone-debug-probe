project("debug_probe")

add_library(${PROJECT_NAME} INTERFACE)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/probe.pio)

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Include/
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/Core/Include/
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(${PROJECT_NAME} INTERFACE
    #${CMAKE_CURRENT_SOURCE_DIR}/debug_probe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/JTAG_DP.c
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP_vendor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/SWO.c
    #${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_DAP/CMSIS/DAP/Firmware/Source/SW_DP.c
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    hardware_pio
)

target_include_directories(${PROJECT_NAME} INTERFACE ${INCLUDES})