#include "settings_cli.h"

#include <cli/args.h>
#include <toolbox/strint.h>
#include "settings.h"

static void settings_cli_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("settings <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\tinfo\t\t\t  - Info settings\r\n");
    printf("\tset_custom_baudrate <0|1> - Set custom baudrate\r\n");
}

static void settings_cli_command_info(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);
    UNUSED(args);

    Settings* settings = furi_record_open(RECORD_SETTINGS);

    printf("uart_custom_baudrate\t%d\r\n", settings_app_is_uart_custom_baudrate_enabled(settings));

    furi_record_close(RECORD_SETTINGS);
}

static void settings_cli_command_set_custom_baudrate(PipeSide* pipe, FuriString* args) {
    UNUSED(pipe);
    UNUSED(args);
    uint8_t baudrate = 0;

    do {
        if(furi_string_size(args) < 1) {
            printf("Missing baudrate argument\r\n");
            break;
        }
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint8(args_cstr, &args_cstr, &baudrate, 10);
        if(parse_err != StrintParseNoError && baudrate <= 1) {
            printf("Invalid baudrate format. Expected a number\r\n");
            break;
        }

        Settings* settings = furi_record_open(RECORD_SETTINGS);
        if(!settings_app_save_uart_custom_baudrate(settings, (bool)baudrate)) {
            printf("Failed to save custom baudrate setting\r\n");
            furi_record_close(RECORD_SETTINGS);
            break;
        }
        furi_record_close(RECORD_SETTINGS);

    } while(false);
}

void settings_cli(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            settings_cli_command_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "info") == 0) {
            settings_cli_command_info(pipe, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "set_custom_baudrate") == 0) {
            settings_cli_command_set_custom_baudrate(pipe, args);
            break;
        }

        settings_cli_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}
