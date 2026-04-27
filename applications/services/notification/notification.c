#include "furi_hal_gpio.h"
#include <furi.h>
#include <furi_hal_resources.h>
#include <debug_probe/debug_probe.h>

#define TAG "NotificationSrv"

typedef enum {
    NotificationThreadFlagDebugProbeIdle = (1 << 0),
    NotificationThreadFlagDebugProbeProcess = (1 << 1),
    NotificationThreadFlagAll = (NotificationThreadFlagDebugProbeIdle | NotificationThreadFlagDebugProbeProcess),
} NotificationThreadFlag;

void notification_callback(bool process, void* context) {
    FuriThreadId thread_id = (FuriThreadId)context;
    furi_thread_flags_set(thread_id, process ? NotificationThreadFlagDebugProbeProcess : NotificationThreadFlagDebugProbeIdle);
}

int32_t notification_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting Notification service");
    furi_hal_gpio_init_simple(&gpio_led, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_led, true);
    const FuriThreadId thread_id = furi_thread_get_current_id();
    debug_probe_set_callback_process(notification_callback, thread_id);
    while(1) {
        uint32_t flags = furi_thread_flags_wait(NotificationThreadFlagAll, FuriFlagWaitAny, FuriWaitForever);

        if(flags & NotificationThreadFlagDebugProbeIdle) {
            furi_hal_gpio_write(&gpio_led, false);
            furi_delay_ms(10);
        }
        if(flags & NotificationThreadFlagDebugProbeProcess) {
            furi_hal_gpio_write(&gpio_led, true);
        }
    }

    furi_crash("Notification service thread exited unexpectedly");
    return 0;
}
