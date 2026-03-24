#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_pio_init(uint32_t baud_rate);
void uart_pio_deinit(void);
void uart_pio_set_baud_rate(uint32_t baud_rate);
uint32_t uart_pio_get_baud_rate(void);
size_t uart_pio_bloking_tx(const uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif
