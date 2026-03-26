#include "furi_hal_gpio.h"
#include <furi.h>
#include <furi_hal_resources.h>

#define TAG "NotificationSrv"

int32_t notification_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting Notification service");
    furi_hal_gpio_init_simple(&gpio_led, GpioModeOutputPushPull);
    furi_hal_gpio_write(&gpio_led, true);
    while (1)
    {
        //furi_hal_gpio_write(&gpio_led, !furi_hal_gpio_read(&gpio_led));
        furi_delay_ms(25000);
    }
    
    furi_crash("Notification service thread exited unexpectedly");
    return 0;
}
