/**
 * @file furi_hal_resources.h
 * @brief Hardware resources API
 */
#pragma once

#include <furi.h>
#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const GpioPin* pin;
    const char* name;
    const uint8_t number;
    const bool debug;
} GpioPinRecord;

extern const GpioPin gpio_uart0_tx;
extern const GpioPin gpio_uart0_rx;

extern const GpioPin gpio_uart1_tx;
extern const GpioPin gpio_uart1_rx;

extern const GpioPin gpio_mcu_swclk;
extern const GpioPin gpio_mcu_swdio;

extern const GpioPin gpio_cpu_reset;
extern const GpioPin gpio_mcu_reset;

extern const GpioPin gpio_cpu_d3;
extern const GpioPin gpio_cpu_d2;

extern const GpioPin gpio_mcu_m40;
extern const GpioPin gpio_mcu_m41;

void furi_hal_resources_init_early(void);

void furi_hal_resources_deinit_early(void);

void furi_hal_resources_init(void);

#ifdef __cplusplus
}
#endif
