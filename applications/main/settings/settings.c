#include "settings.h"
#include <furi.h>
#include <furi_hal_nvm.h>

struct Settings {
    bool uart_set_custom_baudrate;
};


static void settings_app_load_uart_custom_baudrate(Settings* instance) {
    furi_check(instance);
    if(furi_hal_nvm_get_bool(SETTINGS_UART_SET_BAUDRATE, &instance->uart_set_custom_baudrate) != FuriHalNvmStorageOK) {
        instance->uart_set_custom_baudrate = false;
    }
}

bool settings_app_save_uart_custom_baudrate(Settings* instance, bool enable) {
    furi_check(instance);
    if(furi_hal_nvm_set_bool(SETTINGS_UART_SET_BAUDRATE, enable) != FuriHalNvmStorageOK) {
        return false;
    }
    instance->uart_set_custom_baudrate = enable;
    return true;
}

bool settings_app_is_uart_custom_baudrate_enabled(Settings* instance) {
    furi_check(instance);
    return instance->uart_set_custom_baudrate;
}

int32_t settings_app(void* p) {
    Settings* instance = malloc(sizeof(Settings));

    settings_app_load_uart_custom_baudrate(instance);

    furi_record_create(RECORD_SETTINGS, instance);
    return 0;
}
