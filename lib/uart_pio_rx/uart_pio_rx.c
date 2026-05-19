#include "uart_pio_rx.h"
#include "core/common_defines.h"
#include "hardware/pio.h"
#include "uart_rx.pio.h"

#define DEFAULT_BUF_SIZE 1024 * 16

typedef struct {
    const GpioPin* gpio_rx;
    PIO pio;
    uint sm;
    uint offset;
    uint32_t baud_rate;
    int8_t pio_irq;
    uint irq_index;
    FuriStreamBuffer* rx_stream;
    UartPioRxCallback rx_callback;
    void* rx_context;
} UartPioRx;

static UartPioRx* uart_pio_instance = NULL;

static void uart_pio_rx_irq_func(void) {
    while(!pio_sm_is_rx_fifo_empty(uart_pio_instance->pio, uart_pio_instance->sm)) {
        char c = uart_rx_program_getc(uart_pio_instance->pio, uart_pio_instance->sm);
        if(furi_stream_buffer_spaces_available(uart_pio_instance->rx_stream) > 0) {
            furi_check(furi_stream_buffer_send(uart_pio_instance->rx_stream, (const uint8_t*)&c, 1, 0) == 1);
            if(uart_pio_instance->rx_callback) {
                uart_pio_instance->rx_callback(uart_pio_instance->rx_context);
            }
        } else {
            // Buffer is full, drop the character
        }
    }
}

void uart_pio_rx_init(uint32_t baud_rate, const GpioPin* gpio_rx) {
    furi_check(uart_pio_instance == NULL);
    uart_pio_instance = malloc(sizeof(UartPioRx));
    uart_pio_instance->gpio_rx = gpio_rx;
    uart_pio_instance->baud_rate = baud_rate;
    uart_pio_instance->rx_callback = NULL;
    uart_pio_instance->rx_context = NULL;
    uart_pio_instance->rx_stream = furi_stream_buffer_alloc(DEFAULT_BUF_SIZE, 1);

    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
        &uart_rx_program, &uart_pio_instance->pio, &uart_pio_instance->sm, &uart_pio_instance->offset, uart_pio_instance->gpio_rx->pin, 1, true);
    uart_rx_program_init(uart_pio_instance->pio, uart_pio_instance->sm, uart_pio_instance->offset, uart_pio_instance->gpio_rx->pin, baud_rate);

    // Find a free irq
    uart_pio_instance->pio_irq = pio_get_irq_num(uart_pio_instance->pio, 0);
    if(irq_get_exclusive_handler(uart_pio_instance->pio_irq)) {
        uart_pio_instance->pio_irq++;
        if(irq_get_exclusive_handler(uart_pio_instance->pio_irq)) {
            panic("All IRQs are in use");
        }
    }

    // Enable interrupt
    irq_add_shared_handler(uart_pio_instance->pio_irq, uart_pio_rx_irq_func, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
    irq_set_enabled(uart_pio_instance->pio_irq, true); // Enable the IRQ
    uart_pio_instance->irq_index = uart_pio_instance->pio_irq - pio_get_irq_num(uart_pio_instance->pio, 0); // Get index of the IRQ
    pio_set_irqn_source_enabled(
        uart_pio_instance->pio,
        uart_pio_instance->irq_index,
        pio_get_rx_fifo_not_empty_interrupt_source(uart_pio_instance->sm),
        true); // Set pio to tell us when the FIFO is NOT empty
}

void uart_pio_rx_deinit(void) {
    furi_check(uart_pio_instance != NULL);

    // Disable interrupt
    pio_set_irqn_source_enabled(uart_pio_instance->pio, uart_pio_instance->irq_index, pio_get_rx_fifo_not_empty_interrupt_source(uart_pio_instance->sm), false);
    irq_set_enabled(uart_pio_instance->pio_irq, false);
    irq_remove_handler(uart_pio_instance->pio_irq, uart_pio_rx_irq_func);

    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&uart_rx_program, uart_pio_instance->pio, uart_pio_instance->sm, uart_pio_instance->offset);

    furi_stream_buffer_free(uart_pio_instance->rx_stream);
    free(uart_pio_instance);
    uart_pio_instance = NULL;
}

void uart_pio_rx_set_baud_rate(uint32_t baud_rate) {
    furi_check(uart_pio_instance != NULL);

    UartPioRxCallback old_callback = uart_pio_instance->rx_callback;
    void* old_context = uart_pio_instance->rx_context;
    
    uart_pio_rx_deinit();
    uart_pio_rx_init(baud_rate, uart_pio_instance->gpio_rx);
    uart_pio_instance->baud_rate = baud_rate;

    uart_pio_rx_set_callback(old_callback, old_context);
}

uint32_t uart_pio_rx_get_baud_rate(void) {
    furi_check(uart_pio_instance != NULL);
    return uart_pio_instance->baud_rate;
}

void uart_pio_rx_set_callback(UartPioRxCallback callback, void* context) {
    furi_check(uart_pio_instance != NULL);
    furi_check(uart_pio_instance->rx_callback == NULL && uart_pio_instance->rx_context == NULL);
    FURI_CRITICAL_ENTER();
    uart_pio_instance->rx_callback = callback;
    uart_pio_instance->rx_context = context;
    FURI_CRITICAL_EXIT();
}

size_t uart_pio_rx_read(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_check(uart_pio_instance != NULL);
    return furi_stream_buffer_receive(uart_pio_instance->rx_stream, buffer, size, timeout);
}
