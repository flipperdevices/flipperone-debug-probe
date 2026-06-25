#pragma once
#include <stdbool.h>

#define RECORD_SETTINGS            "settings"
#define SETTINGS_UART_SET_BAUDRATE "SettingsUartSetBaudrate"

typedef struct Settings Settings;

#ifdef __cplusplus
extern "C" {
#endif

bool settings_app_save_uart_custom_baudrate(Settings* instance, bool enable);
bool settings_app_is_uart_custom_baudrate_enabled(Settings* instance);

#ifdef __cplusplus
}
#endif
