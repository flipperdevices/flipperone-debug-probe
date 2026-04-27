#pragma once
#include <tusb_edpt_handler.h>

#ifdef __cplusplus
extern "C" {
#endif

void debug_probe_init(void);
void debug_probe_dap_start_thread(void);
void debug_probe_set_callback_process(DapCallbackProcess callback, void* context);

#ifdef __cplusplus
}
#endif
