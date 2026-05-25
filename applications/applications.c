#include "furi.h"
#include "applications.h"
const char* FLIPPER_AUTORUN_APP_NAME = "";

// services
extern int32_t usb_srv(void* p);
extern int32_t cli_vcp_srv(void* p);
extern int32_t uart0_to_cdc_app(void* p);
extern int32_t uart1_to_cdc_app(void* p);
extern int32_t dap_srv(void* p);
extern int32_t notification_srv(void* p);
extern int32_t pio_debug_rx_to_cdc_app(void* p);

// applications
extern int32_t cli_on_system_start(void* p);

// CLI commands

const FlipperInternalApplication FLIPPER_SERVICES[] = {
    {
        .app = usb_srv,
        .name = "UsbSrv",
        .appid = "usb_srv",
        .stack_size = 1024,
        .flags = FlipperInternalApplicationFlagDefault,
    },
    {
        .app = cli_vcp_srv,
        .name = "CliVcpSrv",
        .appid = "cli_vcp_srv",
        .stack_size = 1024 * 2,
        .flags = FlipperInternalApplicationFlagDefault,
    },
    {
        .app = uart0_to_cdc_app,
        .name = "Uart0ToCdc",
        .appid = "uart0_to_cdc",
        .stack_size = 2048,
        .flags = FlipperInternalApplicationFlagDefault,
    },
    {
        .app = uart1_to_cdc_app,
        .name = "Uart1ToCdc",
        .appid = "uart1_to_cdc",
        .stack_size = 2048,
        .flags = FlipperInternalApplicationFlagDefault,
    },
    {
        .app = pio_debug_rx_to_cdc_app,
        .name = "PioDebugRx",
        .appid = "pio_debug_rx",
        .stack_size = 2048,
        .flags = FlipperInternalApplicationFlagDefault,
    },
    {
        .app = dap_srv,
        .name = "DapSrv",
        .appid = "dap_srv",
        .stack_size = 2048,
        .flags = FlipperInternalApplicationFlagDefault,
    },
    {
        .app = notification_srv,
        .name = "NotificationSrv",
        .appid = "notification_srv",
        .stack_size = 1024,
        .flags = FlipperInternalApplicationFlagDefault,
    },
};
const size_t FLIPPER_SERVICES_COUNT = COUNT_OF(FLIPPER_SERVICES);

const FlipperInternalApplication FLIPPER_APPS[] = {

};

const size_t FLIPPER_APPS_COUNT = COUNT_OF(FLIPPER_APPS);

const FlipperInternalApplication FLIPPER_AUTORUN_APPS[] = {
    {
        .app = cli_on_system_start,
        .name = "CliOnSystemStart",
        .appid = "cli_on_system_start",
        .stack_size = 1024 * 2,
        .flags = FlipperInternalApplicationFlagDefault,
    },
};
const size_t FLIPPER_AUTORUN_APPS_COUNT = COUNT_OF(FLIPPER_AUTORUN_APPS);

const FlipperInternalCommandApplication FLIPPER_CLI_COMMANDS[] = {

};
const size_t FLIPPER_CLI_COMMANDS_COUNT = COUNT_OF(FLIPPER_CLI_COMMANDS);
