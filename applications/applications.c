#include "furi.h"
#include "applications.h"
const char* FLIPPER_AUTORUN_APP_NAME = "";

// services
extern int32_t test_peref_srv(void* p);
extern int32_t uart_echo_app(void* p);
extern int32_t usb_srv(void* p);
extern int32_t cli_srv(void* p);
// applications

// CLI commands

const FlipperInternalApplication FLIPPER_SERVICES[] = {
    // {
    //     .app = uart_echo_app,
    //     .name = "UartEcho",
    //     .appid = "uart_echo",
    //     .stack_size = 2048,
    //     .flags = FlipperInternalApplicationFlagDefault,
    // },
    {
        .app = test_peref_srv,
        .name = "TestPerefSrv",
        .appid = "test_peref_srv",
        .stack_size = 1024,
        .flags = FlipperInternalApplicationFlagDefault,
    },
    {
        .app = usb_srv,
        .name = "UsbSrv",
        .appid = "usb_srv",
        .stack_size = 1024,
        .flags = FlipperInternalApplicationFlagDefault,
    },
        {
        .app = cli_srv,
        .name = "CliSrv",
        .appid = "cli_srv",
        .stack_size = 1024 * 2,
        .flags = FlipperInternalApplicationFlagDefault,
    },
};
const size_t FLIPPER_SERVICES_COUNT = COUNT_OF(FLIPPER_SERVICES);

const FlipperInternalApplication FLIPPER_APPS[] = {

};

const size_t FLIPPER_APPS_COUNT = COUNT_OF(FLIPPER_APPS);

const FlipperInternalCommandApplication FLIPPER_CLI_COMMANDS[] = {

};
const size_t FLIPPER_CLI_COMMANDS_COUNT = COUNT_OF(FLIPPER_CLI_COMMANDS);
