#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <tusb.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*tx_ep_callback)(void* context);
    void (*rx_ep_callback)(void* context);
    void (*state_callback)(void* context, uint8_t state);
    void (*ctrl_line_callback)(void* context, uint8_t state);
    void (*config_callback)(void* context, cdc_line_coding_t* config);
} CdcCallbacks;

void furi_hal_usb_cdc_init();

void furi_hal_usb_cdc_deinit();

void furi_hal_cdc_set_callbacks(uint8_t if_num, CdcCallbacks* cb, void* context);

cdc_line_coding_t* furi_hal_cdc_get_port_settings(uint8_t if_num);

uint8_t furi_hal_cdc_get_ctrl_line_state(uint8_t if_num);

/**
 * Send data over a CDC interface, blocking until it is handed to the USB stack.
 *
 * @returns true if all `len` bytes were queued, false if the function gave up via its own
 *          internal timeout (e.g. the host stopped polling the IN endpoint mid-transfer).
 *          On false, the completion callback (tud_cdc_tx_complete_cb / CdcCallbacks::tx_ep_callback)
 *          for this transfer is not guaranteed to ever fire - callers that pace further
 *          sends on that callback must not wait for it in this case.
 */
bool furi_hal_cdc_send(uint8_t if_num, uint8_t* buf, uint16_t len);

int32_t furi_hal_cdc_receive(uint8_t if_num, uint8_t* buf, uint16_t max_len);

#ifdef __cplusplus
}
#endif
