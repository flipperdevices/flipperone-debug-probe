#include <furi_hal_resources.h>

void furi_hal_resources_init(void) {
    furi_hal_gpio_init_simple(&gpio_mcu_m40, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_mcu_m41, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_cpu_d3, GpioModeInput);
    furi_hal_gpio_init_simple(&gpio_cpu_d2, GpioModeInput);

    furi_hal_gpio_init_simple(&gpio_cpu_reset, GpioModeOutputOpenDrain);
    furi_hal_gpio_init_simple(&gpio_mcu_reset, GpioModeOutputOpenDrain);
    furi_hal_gpio_write_open_drain(&gpio_cpu_reset, true);
    furi_hal_gpio_write_open_drain(&gpio_mcu_reset, true);
}

void furi_hal_resources_init_early(void) {
}

void furi_hal_resources_deinit_early(void) {
    // Set all pins to input (as far as SIO is concerned)
    gpio_set_dir_all_bits(0);
    for(int i = 2; i < NUM_BANK0_GPIOS; ++i) {
        gpio_set_function(i, (gpio_function_t)GpioAltFnUnused);
        if(i > NUM_BANK0_GPIOS - NUM_ADC_CHANNELS) {
            gpio_disable_pulls(i);
            gpio_set_input_enabled(i, false);
        }
    }
}
