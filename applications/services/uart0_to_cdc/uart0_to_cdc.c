#include "furi_hal_serial.h"
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb_cdc.h>
#include <settings/settings.h>
#include <cli/cli_ansi.h>

#define TAG "Uart0ToCdc"

#define UART0_TO_CDC_PKT_LEN_RX (CFG_TUD_CDC_RX_BUFSIZE)
#define UART0_TO_CDC_PKT_LEN_TX (CFG_TUD_CDC_RX_BUFSIZE - 1) //Todo: 2 txdone, when sending a full 64-byte packet
#define UART0_TO_CDC_IF_NUM     0
#define DEFAULT_BUF_SIZE        (1024 * 16)

#define DEFAULT_BAUD_RATE (1500000UL)
#define DEFAULT_DATA_BITS FuriHalSerialConfigDataBits8
#define DEFAULT_PARITY    FuriHalSerialConfigParityNone
#define DEFAULT_STOP_BITS FuriHalSerialConfigStopBits_1

#define DEFAULT_LOG_MESSAGE "Flipper One CPU console, fixed baud rate:"

//#define UART0_TO_CDC_DEBUG

#ifdef UART0_TO_CDC_DEBUG
#define UART0_TO_CDC_LOG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define UART0_TO_CDC_LOG(...)
#endif

static void uart0_to_cdc_tx_complete(void* context);
static void uart0_to_cdc_rx(void* context);
static void uart0_to_cdc_state_callback(void* context, uint8_t state);
static void uart0_to_cdc_control_line(void* context, uint8_t state);
static void uart0_to_cdc_config(void* context, cdc_line_coding_t* config);

static CdcCallbacks uart0_to_cdc_cb = {
    uart0_to_cdc_tx_complete,
    uart0_to_cdc_rx,
    uart0_to_cdc_state_callback,
    uart0_to_cdc_control_line,
    uart0_to_cdc_config,
};

typedef struct {
    FuriThread* thread;
    FuriStreamBuffer* rx_stream;
    FuriStreamBuffer* tx_stream;
    FuriHalSerialHandle* serial_handle;
    bool cdc_tx_idle;
    uint8_t data_buffer[UART0_TO_CDC_PKT_LEN_RX];
    bool connected;
    uint32_t baudrate;
    Settings* settings;
} Uart0ToCdcApp;

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

static void uart0_to_cdc_tx_complete(void* context) {
    Uart0ToCdcApp* instance = context;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcTx);
}

static void uart0_to_cdc_rx(void* context) {
    Uart0ToCdcApp* instance = context;
    uint32_t ret = furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcRx);
    furi_check(!(ret & FuriFlagError));
}

static void uart0_to_cdc_state_callback(void* context, uint8_t state) {
    Uart0ToCdcApp* instance = context;
    if(state == 0) {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcDisconnect);
    }
}

static void uart0_to_cdc_control_line(void* context, uint8_t state) {
    Uart0ToCdcApp* instance = context;
    // bit 0: DTR state, bit 1: RTS state
    bool dtr = state & (1 << 0);

    if(dtr == true) {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcConnect);
    } else {
        furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcDisconnect);
    }
}

static void uart0_to_cdc_config(void* context, cdc_line_coding_t* config) {
    Uart0ToCdcApp* instance = context;
    instance->baudrate = config->bit_rate;
    furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventCdcConfig);
}

static int32_t uart0_to_cdc_worker(void* context) {
    furi_assert(context);
    Uart0ToCdcApp* instance = context;
    FURI_LOG_D(TAG, "Start");

    furi_hal_cdc_set_callbacks(UART0_TO_CDC_IF_NUM, &uart0_to_cdc_cb, instance);
    size_t missed_rx = 0;
    size_t length = 0;
    while(1) {
        uint32_t events = furi_thread_flags_wait(WORKER_EVENTS_MASK, FuriFlagWaitAny, FuriWaitForever);

        if(events & WorkerEventUartTxComplete) {
            if(missed_rx) {
                events |= WorkerEventCdcRx;
                missed_rx--;
                UART0_TO_CDC_LOG("UART Tx complete, missed Rx %d", missed_rx);
            }
            events |= WorkerEventUartTx;
        }

        if(events & WorkerEventCdcRx) {
            if(furi_stream_buffer_spaces_available(instance->rx_stream) >= UART0_TO_CDC_PKT_LEN_RX) {
                length = furi_hal_cdc_receive(UART0_TO_CDC_IF_NUM, instance->data_buffer, UART0_TO_CDC_PKT_LEN_RX);
                UART0_TO_CDC_LOG("Rx %d", length);

                if(length > 0) {
                    furi_check(furi_stream_buffer_send(instance->rx_stream, instance->data_buffer, length, FuriWaitForever) == (size_t)length);
                    events |= WorkerEventUartTx;
                }
            } else {
                FURI_LOG_W(TAG, "Rx missed");
                missed_rx++;
            }
        }

        if(events & WorkerEventUartRx) {
            if(instance->cdc_tx_idle) {
                events |= WorkerEventCdcTx;
                UART0_TO_CDC_LOG("UART Rx trigger CDC Tx");
            }
        }

        if(events & WorkerEventCdcTx) {
            length = furi_stream_buffer_receive(instance->tx_stream, instance->data_buffer, UART0_TO_CDC_PKT_LEN_TX, 0);
            UART0_TO_CDC_LOG("UART Tx %d", length);
            if(length > 0) {
                if(instance->connected) {
                    instance->cdc_tx_idle = false;
                    furi_hal_cdc_send(UART0_TO_CDC_IF_NUM, instance->data_buffer, length);
                }
            } else {
                //furi_hal_cdc_send(UART0_TO_CDC_IF_NUM, NULL, 0);
                instance->cdc_tx_idle = true;
            }
        }

        if(events & WorkerEventUartTx) {
            UART0_TO_CDC_LOG("UART Tx start");
            uint8_t data;
            while(furi_hal_serial_tx_ready(instance->serial_handle)) {
                length = furi_stream_buffer_receive(instance->rx_stream, &data, 1, 0);
                if(length > 0) {
                    furi_hal_serial_tx_non_blocking(instance->serial_handle, data);
                } else {
                    UART0_TO_CDC_LOG("UART Tx idle");
                    break;
                }
            }
        }

        if(events & WorkerEventCdcConnect) {
            UART0_TO_CDC_LOG("CDC connected");
            if(!settings_app_is_uart_custom_baudrate_enabled(instance->settings) && !instance->connected) {
                UART0_TO_CDC_LOG("CDC connected, send default log message");
                uint8_t buf[128];
                int32_t length =
                    snprintf((char*)buf, sizeof(buf), ANSI_BG_WHITE ANSI_FG_BR_BLACK "\r\n%s %ld\r\n" ANSI_RESET, DEFAULT_LOG_MESSAGE, DEFAULT_BAUD_RATE);
                furi_delay_ms(33);
                furi_hal_cdc_send(UART0_TO_CDC_IF_NUM, buf, length);
                furi_hal_serial_tx_non_blocking(instance->serial_handle, CliKeyETX); // send Ctrl+C to CPU console
            }
            instance->connected = true;
        }

        if(events & WorkerEventCdcDisconnect) {
            UART0_TO_CDC_LOG("CDC disconnected");
            instance->connected = false;
            furi_stream_buffer_reset(instance->tx_stream);
            furi_stream_buffer_reset(instance->rx_stream);
        }

        if(events & WorkerEventError) {
            FURI_LOG_E(TAG, "WorkerEventError");
        }

        if(events & WorkerEventCdcConfig) {
            if(settings_app_is_uart_custom_baudrate_enabled(instance->settings)) {
                UART0_TO_CDC_LOG("CDC config changed");
                furi_hal_serial_set_baud_rate(instance->serial_handle, instance->baudrate);
                UART0_TO_CDC_LOG("CDC config baud rate %ld", instance->baudrate);
            } else {
                UART0_TO_CDC_LOG("CDC config changed, but custom baudrate is disabled");
            }
        }

        if(events & WorkerEventStop) break;
    }

    furi_hal_cdc_set_callbacks(UART0_TO_CDC_IF_NUM, NULL, NULL);
    FURI_LOG_D(TAG, "Stop");
    return 0;
}

static void uart0_to_cdc_on_irq_cb(FuriHalSerialHandle* handle, FuriHalSerialRxEvent event, void* context) {
    furi_assert(context);
    UNUSED(handle);
    Uart0ToCdcApp* instance = context;
    WorkerEventFlags flag = 0;

    uint8_t data[UART0_TO_CDC_PKT_LEN_TX];
    size_t length = 0;
    size_t buf_size_rx = 0;
    if(event & (FuriHalSerialRxEventData | FuriHalSerialRxEventIdle)) {
        length = furi_hal_serial_rx_data_non_blocking(handle, data, UART0_TO_CDC_PKT_LEN_TX);
        if(instance->connected) {
            buf_size_rx = furi_stream_buffer_send(instance->tx_stream, &data, length, 0);
            if(buf_size_rx != length) {
                flag |= WorkerEventError;
            } else {
                flag |= WorkerEventUartRx;
            }
        }
    }
    //error detected
    if(event & (FuriHalSerialRxEventFrameError | FuriHalSerialRxEventBreakError | FuriHalSerialRxEventOverrunError | FuriHalSerialRxEventParityError)) {
        flag |= WorkerEventError;
    }

    furi_thread_flags_set(furi_thread_get_id(instance->thread), flag);
}

static void uart0_to_cdc_tx_complete_irq_cb(FuriHalSerialHandle* handle, FuriHalSerialTxEvent event, void* context) {
    UNUSED(handle);
    WorkerEventFlags flag = 0;
    Uart0ToCdcApp* app = context;
    if(event & FuriHalSerialTxEventComplete) {
        flag |= (WorkerEventUartTxComplete);
    }
    furi_thread_flags_set(furi_thread_get_id(app->thread), flag);
}

static Uart0ToCdcApp* uart0_to_cdc_app_alloc(void) {
    furi_check(UART0_TO_CDC_PKT_LEN_RX >= UART0_TO_CDC_PKT_LEN_TX);
    Uart0ToCdcApp* instance = malloc(sizeof(Uart0ToCdcApp));
    instance->rx_stream = furi_stream_buffer_alloc(DEFAULT_BUF_SIZE, 1);
    instance->tx_stream = furi_stream_buffer_alloc(DEFAULT_BUF_SIZE, 1);

    // Enable uart listener
    instance->baudrate = DEFAULT_BAUD_RATE;
    FuriHalSerialConfigDataBits data_bits = DEFAULT_DATA_BITS;
    FuriHalSerialConfigParity parity = DEFAULT_PARITY;
    FuriHalSerialConfigStopBits stop_bits = DEFAULT_STOP_BITS;

    instance->thread = furi_thread_get_current_id();

    instance->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUart0);
    furi_check(instance->serial_handle);
    furi_hal_serial_init(instance->serial_handle, instance->baudrate);
    furi_hal_serial_set_config(instance->serial_handle, data_bits, parity, stop_bits);
    instance->cdc_tx_idle = true;

    furi_hal_serial_set_callback(instance->serial_handle, uart0_to_cdc_tx_complete_irq_cb, uart0_to_cdc_on_irq_cb, instance);
    furi_hal_serial_async_rx_start(instance->serial_handle, true);

    instance->settings = furi_record_open(RECORD_SETTINGS);

    return instance;
}

void uart0_to_cdc_app_free(Uart0ToCdcApp* instance) {
    furi_assert(instance);

    furi_record_close(RECORD_SETTINGS);
    furi_thread_flags_set(furi_thread_get_id(instance->thread), WorkerEventStop);
    furi_hal_serial_async_rx_stop(instance->serial_handle);
    furi_hal_serial_deinit(instance->serial_handle);
    furi_hal_serial_control_release(instance->serial_handle);

    furi_stream_buffer_free(instance->rx_stream);
    furi_stream_buffer_free(instance->tx_stream);

    free(instance);
}

int32_t uart0_to_cdc_app(void* p) {
    UNUSED(p);
    Uart0ToCdcApp* instance = uart0_to_cdc_app_alloc();
    uart0_to_cdc_worker(instance);
    furi_crash("uart0_to_cdc_app exited unexpectedly");
    return 0;
}
