#include <furi.h>
#include <debug_probe/debug_probe.h>

#define TAG "DapSrv"

int32_t dap_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting DAP service");
    debug_probe_init();
    debug_probe_dap_start_thread();
    furi_crash("DAP service thread exited unexpectedly");
    return 0;
}
