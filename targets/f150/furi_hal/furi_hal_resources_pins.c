#include "furi_hal_gpio.h"
#include <furi_hal_resources.h>

const GpioPin gpio_uart0_tx = {.pin = 0};
const GpioPin gpio_uart0_rx = {.pin = 1};

const GpioPin gpio_uart1_tx = {.pin = 4};
const GpioPin gpio_uart1_rx = {.pin = 5};

const GpioPin gpio_mcu_swclk = {.pin = 2};
const GpioPin gpio_mcu_swdio = {.pin = 3};

const GpioPin gpio_cpu_reset = {.pin = 7};
const GpioPin gpio_mcu_reset = {.pin = 8};

const GpioPin gpio_cpu_d3 = {.pin = 10};
const GpioPin gpio_cpu_d2 = {.pin = 11};

const GpioPin gpio_mcu_m40 = {.pin = 12};
const GpioPin gpio_mcu_m41 = {.pin = 13};

const GpioPin gpio_led = {.pin = 25};

const GpioPinRecord gpio_pins[] = {};
const size_t gpio_pins_count = COUNT_OF(gpio_pins);
