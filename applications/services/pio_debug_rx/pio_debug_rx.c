
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb_cdc.h>
#include "uart_pio_rx.h"

#define TAG "PioDebugRx"

#define DEFAULT_BUF_SIZE    1024 * 8
//#define PIO_DEBUG_RX_TO_CDC_IF_NUM 3

#define DEFAULT_BAUD_RATE 230400


#define PIO_DEBUG_RX_TO_CDC_DEBUG

#ifdef PIO_DEBUG_RX_TO_CDC_DEBUG
#define PIO_DEBUG_RX_TO_CDC_LOG(...) FURI_LOG_I(TAG, __VA_ARGS__)
#else
#define PIO_DEBUG_RX_TO_CDC_LOG(...)
#endif

static void pio_debug_rx_to_cdc_tx_complete(void* context);
static void pio_debug_rx_to_cdc_rx(void* context);
static void pio_debug_rx_to_cdc_state_callback(void* context, uint8_t state);
static void pio_debug_rx_to_cdc_control_line(void* context, uint8_t state);
static void pio_debug_rx_to_cdc_config(void* context, cdc_line_coding_t* config);

static CdcCallbacks pio_debug_rx_to_cdc_cb = {
    pio_debug_rx_to_cdc_tx_complete,
    pio_debug_rx_to_cdc_rx,
    pio_debug_rx_to_cdc_state_callback,
    pio_debug_rx_to_cdc_control_line,
    pio_debug_rx_to_cdc_config,
};

typedef struct {
    FuriThread* thread;
    FuriStreamBuffer* rx_stream;
    FuriStreamBuffer* tx_stream;
    //FuriHalSerialHandle* serial_handle;
    bool cdc_tx_idle;
    uint8_t data_buffer[CFG_TUD_CDC_RX_BUFSIZE];
    bool connected;
    uint32_t baudrate;
} PioDebugRxToCdcApp;

typedef enum {
    WorkerEventReserved = (1 << 0),
    WorkerEventStop = (1 << 1),
    WorkerEventCdcRx = (1 << 2),
    WorkerEventCdcTx = (1 << 3),
    WorkerEventUartTxComplete = (1 << 4),
    WorkerEventUartRx = (1 << 5),
    WorkerEventUartTx = (1 << 6),
    WorkerEventCdcConnect = (1 << 7),
    WorkerEventCdcDisconnect = (1 << 8),
    WorkerEventCdcConfig = (1 << 9),
    WorkerEventError = (1 << 10),

} WorkerEventFlags;

#define WORKER_EVENTS_MASK                                                                                                                               \
    (WorkerEventStop | WorkerEventCdcRx | WorkerEventCdcTx | WorkerEventUartTxComplete | WorkerEventUartRx | WorkerEventUartTx | WorkerEventCdcConnect | \
     WorkerEventCdcDisconnect | WorkerEventCdcConfig | WorkerEventError)

static void pio_debug_rx_to_cdc_tx_complete(void* context) {
    PioDebugRxToCdcApp* instance = context;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcTx);
}

static void pio_debug_rx_to_cdc_rx(void* context) {
    PioDebugRxToCdcApp* instance = context;
    uint32_t ret = furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcRx);
    furi_check(!(ret & FuriFlagError));
}

static void pio_debug_rx_to_cdc_state_callback(void* context, uint8_t state) {
    PioDebugRxToCdcApp* instance = context;
    if(state == 0) {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcDisconnect);
    }
}

static void pio_debug_rx_to_cdc_control_line(void* context, uint8_t state) {
    PioDebugRxToCdcApp* instance = context;
    // bit 0: DTR state, bit 1: RTS state
    bool dtr = state & (1 << 0);

    if(dtr == true) {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcConnect);
    } else {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcDisconnect);
    }
}

static void pio_debug_rx_to_cdc_config(void* context, cdc_line_coding_t* config) {
    PioDebugRxToCdcApp* instance = context;
    instance->baudrate = config->bit_rate;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcConfig);
}

static int32_t pio_debug_rx_to_cdc_worker(void* context) {
    furi_assert(context);
    PioDebugRxToCdcApp* instance = context;
    FURI_LOG_D(TAG, "Start");

    //furi_hal_cdc_set_callbacks(PIO_DEBUG_RX_TO_CDC_IF_NUM, &pio_debug_rx_to_cdc_cb, instance);
    size_t missed_rx = 0;
    size_t length = 0;
    while(1) {
        uint32_t events = furi_thread_flags_wait(WORKER_EVENTS_MASK, FuriFlagWaitAny, FuriWaitForever);
        furi_delay_us(100);
        // if(events & WorkerEventUartTxComplete) {
        //     if(missed_rx) {
        //         events |= WorkerEventCdcRx;
        //         missed_rx--;
        //         PIO_DEBUG_RX_TO_CDC_LOG("UART Tx complete, missed Rx %d", missed_rx);
        //     }
        //     events |= WorkerEventUartTx;
        // }

        // if(events & WorkerEventCdcRx) {
        //     if(furi_stream_buffer_spaces_available(instance->rx_stream) >= CFG_TUD_CDC_RX_BUFSIZE) {
        //         length = furi_hal_cdc_receive(PIO_DEBUG_RX_TO_CDC_IF_NUM, instance->data_buffer, CFG_TUD_CDC_RX_BUFSIZE);
        //         PIO_DEBUG_RX_TO_CDC_LOG("Rx %d", length);

        //         if(length > 0) {
        //             furi_check(furi_stream_buffer_send(instance->rx_stream, instance->data_buffer, length, FuriWaitForever) == (size_t)length);
        //             events |= WorkerEventUartTx;
        //         }
        //     } else {
        //         FURI_LOG_W(TAG, "Rx missed");
        //         missed_rx++;
        //     }
        // }

        if(events & WorkerEventUartRx) {
            if(instance->cdc_tx_idle) {
                events |= WorkerEventCdcTx;
                PIO_DEBUG_RX_TO_CDC_LOG("UART Rx trigger CDC Tx");
            }
        }

        if(events & WorkerEventCdcTx) {
            //length = furi_stream_buffer_receive(instance->tx_stream, instance->data_buffer, CFG_TUD_CDC_RX_BUFSIZE, 0);
            length = uart_pio_rx_read(instance->data_buffer, CFG_TUD_CDC_RX_BUFSIZE, 0);
            PIO_DEBUG_RX_TO_CDC_LOG("UART Tx %d", length);
            PIO_DEBUG_RX_TO_CDC_LOG("UART Tx %s", instance->data_buffer);
            // if(length > 0) {
            //     instance->cdc_tx_idle = false;
            //     if(instance->connected) {
            //         furi_hal_cdc_send(PIO_DEBUG_RX_TO_CDC_IF_NUM, instance->data_buffer, length);
            //     }
            // } else {
            //     //furi_hal_cdc_send(PIO_DEBUG_RX_TO_CDC_IF_NUM, NULL, 0);
            //     instance->cdc_tx_idle = true;
            // }
        }

        // if(events & WorkerEventUartTx) {
        //     PIO_DEBUG_RX_TO_CDC_LOG("UART Tx start");
        //     uint8_t data;
        //     while(furi_hal_serial_tx_ready(instance->serial_handle)) {
        //         length = furi_stream_buffer_receive(instance->rx_stream, &data, 1, 0);
        //         if(length > 0) {
        //             furi_hal_serial_tx_non_blocking(instance->serial_handle, data);
        //         } else {
        //             PIO_DEBUG_RX_TO_CDC_LOG("UART Tx idle");
        //             break;
        //         }
        //     }
        // }

        // if(events & WorkerEventCdcConnect) {
        //     PIO_DEBUG_RX_TO_CDC_LOG("CDC connected");
        //     instance->connected = true;
        // }

        // if(events & WorkerEventCdcDisconnect) {
        //     PIO_DEBUG_RX_TO_CDC_LOG("CDC disconnected");
        //     instance->connected = false;
        //     furi_stream_buffer_reset(instance->tx_stream);
        //     furi_stream_buffer_reset(instance->rx_stream);
        // }

        // if(events & WorkerEventError) {
        //     FURI_LOG_E(TAG, "WorkerEventError");
        // }

        // if(events & WorkerEventCdcConfig) {
        //     PIO_DEBUG_RX_TO_CDC_LOG("CDC config changed");
        //     furi_hal_serial_set_baud_rate(instance->serial_handle, instance->baudrate);
        //     PIO_DEBUG_RX_TO_CDC_LOG("CDC config baud rate %ld", instance->baudrate);
        // }

        if(events & WorkerEventStop) break;
        furi_delay_us(100);
    }

    //furi_hal_cdc_set_callbacks(PIO_DEBUG_RX_TO_CDC_IF_NUM, NULL, NULL);
    FURI_LOG_D(TAG, "Stop");
    return 0;
}

// static void pio_debug_rx_to_cdc_on_irq_cb(FuriHalSerialHandle* handle, FuriHalSerialRxEvent event, void* context) {
//     furi_assert(context);
//     UNUSED(handle);
//     PioDebugRxToCdcApp* instance = context;
//     WorkerEventFlags flag = 0;

//     uint8_t data[CFG_TUD_CDC_RX_BUFSIZE];
//     size_t length = 0;
//     size_t buf_size_rx = 0;
//     if(event & (FuriHalSerialRxEventData | FuriHalSerialRxEventIdle)) {
//         length = furi_hal_serial_rx_data_non_blocking(handle, data, CFG_TUD_CDC_RX_BUFSIZE);
//         if(instance->connected) {
//             buf_size_rx = furi_stream_buffer_send(instance->tx_stream, &data, length, 0);
//             if(buf_size_rx != length) {
//                 flag |= WorkerEventError;
//             } else {
//                 flag |= WorkerEventUartRx;
//             }
//         }
//     }
//     //error detected
//     if(event & (FuriHalSerialRxEventFrameError | FuriHalSerialRxEventBreakError | FuriHalSerialRxEventOverrunError | FuriHalSerialRxEventParityError)) {
//         flag |= WorkerEventError;
//     }

//     furi_thread_flags_set(furi_thread_get_id(instance->thread), flag);
// }

// static void pio_debug_rx_to_cdc_tx_complete_irq_cb(FuriHalSerialHandle* handle, FuriHalSerialTxEvent event, void* context) {
//     UNUSED(handle);
//     WorkerEventFlags flag = 0;
//     PioDebugRxToCdcApp* app = context;
//     if(event & FuriHalSerialTxEventComplete) {
//         flag |= (WorkerEventUartTxComplete);
//     }
//     furi_thread_flags_set(furi_thread_get_id(app->thread), flag);
// }

static void pio_debug_rx_isr_callback(void* context) {
    PioDebugRxToCdcApp* instance = context;
    WorkerEventFlags flag = 0;
    flag |= WorkerEventUartRx;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), flag);
}

static PioDebugRxToCdcApp* pio_debug_rx_to_cdc_app_alloc(void) {
    PioDebugRxToCdcApp* instance = malloc(sizeof(PioDebugRxToCdcApp));
    instance->rx_stream = furi_stream_buffer_alloc(DEFAULT_BUF_SIZE, 1);
    instance->tx_stream = furi_stream_buffer_alloc(DEFAULT_BUF_SIZE, 1);

    // Enable uart listener
    instance->baudrate = DEFAULT_BAUD_RATE;

    instance->thread = furi_thread_get_current_id();

    // instance->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUart1);
    // furi_check(instance->serial_handle);
    // furi_hal_serial_init(instance->serial_handle, instance->baudrate);
    // furi_hal_serial_set_config(instance->serial_handle, data_bits, parity, stop_bits);
    uart_pio_rx_init(instance->baudrate, &gpio_mcu_debug_rx);
    instance->cdc_tx_idle = true;

    uart_pio_rx_set_callback(pio_debug_rx_isr_callback, instance);
    // furi_hal_serial_set_callback(instance->serial_handle, pio_debug_rx_to_cdc_tx_complete_irq_cb, pio_debug_rx_to_cdc_on_irq_cb, instance);
    // furi_hal_serial_async_rx_start(instance->serial_handle, true);

    return instance;
}

void pio_debug_rx_to_cdc_app_free(PioDebugRxToCdcApp* instance) {
    furi_assert(instance);

    furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventStop);

    // furi_hal_serial_async_rx_stop(instance->serial_handle);
    // furi_hal_serial_deinit(instance->serial_handle);
    // furi_hal_serial_control_release(instance->serial_handle);

    uart_pio_rx_deinit();

    furi_stream_buffer_free(instance->rx_stream);
    furi_stream_buffer_free(instance->tx_stream);

    free(instance);
}

int32_t pio_debug_rx_to_cdc_app(void* p) {
    UNUSED(p);
    PioDebugRxToCdcApp* instance = pio_debug_rx_to_cdc_app_alloc();
    pio_debug_rx_to_cdc_worker(instance);
    furi_crash("pio_debug_rx_to_cdc_app exited unexpectedly");
    return 0;
}
