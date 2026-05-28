#include <furi_hal.h>
#include <furi_hal_power.h>
#include <furi_hal_nvm.h>
#include <furi_hal_gpio.h>
#include <furi_hal_cortex.h>

#define TAG "FuriHal"

void furi_hal_init_early(void) {
    furi_hal_cortex_init_early();
    furi_hal_nvm_init();
    // furi_hal_clock_init_early();
    // furi_hal_resources_init_early();
    furi_hal_os_init();
}

void furi_hal_deinit_early(void) {
    // furi_hal_resources_deinit_early();
    // furi_hal_clock_deinit_early();
}

void furi_hal_init(void) {
    // furi_hal_clock_init();
    furi_hal_gpio_interrupt_init();
    furi_hal_serial_control_init();
    // furi_hal_rtc_init();
    // furi_hal_interrupt_init();
    // furi_hal_flash_init();
    furi_hal_resources_init();
    // furi_hal_version_init();
    //furi_hal_power_init();
    //furi_hal_memory_init();
}
