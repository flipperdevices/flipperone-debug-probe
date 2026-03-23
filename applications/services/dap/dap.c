#include <furi.h>
#include <debug_probe/hid_glue.h>

#define TAG "DapSrv"

int32_t dap_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting DAP service");
    hid_glue_init();
    hid_glue_dap_start_thread();

    // We should never reach this point
    while (1)
    {
            furi_delay_ms(1000);
            FURI_LOG_I(TAG, "DAP service running...");
    }
    

    furi_crash();
    return 0;
}
