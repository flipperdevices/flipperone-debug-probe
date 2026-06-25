
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb_cdc.h>
#include <uart_pio_rx.h>

#define TAG "PioDebugRx"

#define PIO_DEBUG_RX_TO_CDC_PKT_LEN_RX (CFG_TUD_CDC_RX_BUFSIZE)
#define PIO_DEBUG_RX_TO_CDC_PKT_LEN_TX (CFG_TUD_CDC_RX_BUFSIZE - 1) //Todo: 2 txdone, when sending a full 64-byte packet
#define PIO_DEBUG_RX_TO_CDC_IF_NUM  2

#define DEFAULT_BAUD_RATE (1500000UL)

//#define PIO_DEBUG_RX_TO_CDC_DEBUG

#ifdef PIO_DEBUG_RX_TO_CDC_DEBUG
#define PIO_DEBUG_RX_TO_CDC_LOG(...) FURI_LOG_D(TAG, __VA_ARGS__)
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
    bool cdc_tx_idle;
    uint8_t data_buffer[PIO_DEBUG_RX_TO_CDC_PKT_LEN_RX];
    bool connected;
    uint32_t baudrate;
} PioDebugRxToCdcApp;

typedef enum {
    WorkerEventReserved = (1 << 0),
    WorkerEventStop = (1 << 1),
    WorkerEventCdcRx = (1 << 2),
    WorkerEventCdcTx = (1 << 3),
    WorkerEventUartRx = (1 << 5),
    WorkerEventCdcConnect = (1 << 7),
    WorkerEventCdcDisconnect = (1 << 8),
    WorkerEventCdcConfig = (1 << 9),
    WorkerEventError = (1 << 10),

} WorkerEventFlags;

#define WORKER_EVENTS_MASK                                                                                                                                 \
    (WorkerEventStop | WorkerEventCdcRx | WorkerEventCdcTx | WorkerEventUartRx | WorkerEventCdcConnect | WorkerEventCdcDisconnect | WorkerEventCdcConfig | \
     WorkerEventError)

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

    furi_hal_cdc_set_callbacks(PIO_DEBUG_RX_TO_CDC_IF_NUM, &pio_debug_rx_to_cdc_cb, instance);
    size_t length = 0;
    while(1) {
        uint32_t events = furi_thread_flags_wait(WORKER_EVENTS_MASK, FuriFlagWaitAny, FuriWaitForever);

        if(events & WorkerEventCdcRx) {
            // receive data to null
            furi_hal_cdc_receive(PIO_DEBUG_RX_TO_CDC_IF_NUM, instance->data_buffer, PIO_DEBUG_RX_TO_CDC_PKT_LEN_RX);
        }

        if(events & WorkerEventUartRx) {
            if(instance->cdc_tx_idle) {
                events |= WorkerEventCdcTx;
                PIO_DEBUG_RX_TO_CDC_LOG("UART Rx trigger CDC Tx");
            }
        }

        if(events & WorkerEventCdcTx) {
            length = uart_pio_rx_read(instance->data_buffer, PIO_DEBUG_RX_TO_CDC_PKT_LEN_TX, 0);
            PIO_DEBUG_RX_TO_CDC_LOG("UART Tx %d", length);
            if(length > 0) {
                if(instance->connected) {
                    instance->cdc_tx_idle = false;
                    furi_hal_cdc_send(PIO_DEBUG_RX_TO_CDC_IF_NUM, instance->data_buffer, length);
                }
            } else {
                //furi_hal_cdc_send(PIO_DEBUG_RX_TO_CDC_IF_NUM, NULL, 0);
                instance->cdc_tx_idle = true;
            }
        }

        if(events & WorkerEventCdcConnect) {
            PIO_DEBUG_RX_TO_CDC_LOG("CDC connected");
            instance->connected = true;
        }

        if(events & WorkerEventCdcDisconnect) {
            PIO_DEBUG_RX_TO_CDC_LOG("CDC disconnected");
            instance->connected = false;
        }

        if(events & WorkerEventError) {
            FURI_LOG_E(TAG, "WorkerEventError");
        }

        if(events & WorkerEventCdcConfig) {
            PIO_DEBUG_RX_TO_CDC_LOG("CDC config changed");
            //Todo: No need update baud rate
            // uart_pio_rx_set_baud_rate(instance->baudrate);
            PIO_DEBUG_RX_TO_CDC_LOG("Note: No need update baud rate, because the baud rate is fixed in the PIO debug rx");
            PIO_DEBUG_RX_TO_CDC_LOG("CDC config baud rate %ld", instance->baudrate);
        }

        if(events & WorkerEventStop) break;
    }

    furi_hal_cdc_set_callbacks(PIO_DEBUG_RX_TO_CDC_IF_NUM, NULL, NULL);
    FURI_LOG_D(TAG, "Stop");
    return 0;
}

static void pio_debug_rx_isr_callback(void* context) {
    PioDebugRxToCdcApp* instance = context;
    WorkerEventFlags flag = 0;
    flag |= WorkerEventUartRx;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), flag);
}

static PioDebugRxToCdcApp* pio_debug_rx_to_cdc_app_alloc(void) {
    furi_check(PIO_DEBUG_RX_TO_CDC_PKT_LEN_RX >= PIO_DEBUG_RX_TO_CDC_PKT_LEN_TX);
    PioDebugRxToCdcApp* instance = malloc(sizeof(PioDebugRxToCdcApp));

    // Enable uart listener
    instance->baudrate = DEFAULT_BAUD_RATE;

    instance->thread = furi_thread_get_current_id();

    uart_pio_rx_init(instance->baudrate, &gpio_mcu_debug_rx);
    instance->cdc_tx_idle = true;

    uart_pio_rx_set_callback(pio_debug_rx_isr_callback, instance);

    return instance;
}

void pio_debug_rx_to_cdc_app_free(PioDebugRxToCdcApp* instance) {
    furi_assert(instance);

    furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventStop);
    uart_pio_rx_deinit();
    free(instance);
}

int32_t pio_debug_rx_to_cdc_app(void* p) {
    UNUSED(p);
    PioDebugRxToCdcApp* instance = pio_debug_rx_to_cdc_app_alloc();
    pio_debug_rx_to_cdc_worker(instance);
    furi_crash("pio_debug_rx_to_cdc_app exited unexpectedly");
    return 0;
}
