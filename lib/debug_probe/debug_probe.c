#include "debug_probe.h"
#include <tusb.h>
#include <DAP.h>
#include <tusb_edpt_handler.h>

static uint8_t TxDataBuffer[CFG_TUD_HID_EP_BUFSIZE];
static uint8_t RxDataBuffer[CFG_TUD_HID_EP_BUFSIZE];

void debug_probe_init(void) {
    DAP_Setup();
}

void debug_probe_dap_start_thread(void) {
    dap_thread(NULL);
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    // TODO not Implemented
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* RxDataBuffer, uint16_t bufsize) {
    uint32_t response_size = TU_MIN(CFG_TUD_HID_EP_BUFSIZE, bufsize);

    // This doesn't use multiple report and report ID
    (void)itf;
    (void)report_id;
    (void)report_type;

    DAP_ProcessCommand(RxDataBuffer, TxDataBuffer);

    tud_hid_report(0, TxDataBuffer, response_size);
}

#if (PROBE_DEBUG_PROTOCOL == PROTO_DAP_V2)
extern uint8_t const desc_ms_os_20[];
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const* request) {
    // nothing to with DATA & ACK stage
    if(stage != CONTROL_STAGE_SETUP) return true;

    switch(request->bmRequestType_bit.type) {
    case TUSB_REQ_TYPE_VENDOR:
        switch(request->bRequest) {
        case 1:
            if(request->wIndex == 7) {
                // Get Microsoft OS 2.0 compatible descriptor
                uint16_t total_len;
                memcpy(&total_len, desc_ms_os_20 + 8, 2);

                return tud_control_xfer(rhport, request, (void*)desc_ms_os_20, total_len);
            } else {
                return false;
            }

        default:
            break;
        }
        break;
    default:
        break;
    }

    // stall unknown request
    return false;
}
#endif
