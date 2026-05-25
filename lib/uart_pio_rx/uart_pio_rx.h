#pragma once

#include <stdint.h>
#include <stddef.h>
#include "furi_hal_resources.h"

typedef void (*UartPioRxCallback)(void* context);

#ifdef __cplusplus
extern "C" {
#endif

void uart_pio_rx_init(uint32_t baud_rate, const GpioPin* gpio_rx);
void uart_pio_rx_deinit(void);
void uart_pio_rx_set_baud_rate(uint32_t baud_rate);
uint32_t uart_pio_rx_get_baud_rate(void);
void uart_pio_rx_set_callback(UartPioRxCallback callback, void* context);
size_t uart_pio_rx_read(uint8_t* buffer, size_t size, uint32_t timeout);
#ifdef __cplusplus
}
#endif
