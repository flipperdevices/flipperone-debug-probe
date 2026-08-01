#include "cli_commands_common.h"

#include <furi_hal.h>
#include <cli/cli_ansi.h>
#include <cli/args.h>
#include <cli/cli_command.h>
#include <time.h>
#include <furi_bsp.h>
#include <furi_hal_clock.h>
#include <furi_hal_otp.h>

void cli_command_uptime(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);
    uint32_t uptime = furi_get_tick() / furi_kernel_get_tick_frequency();
    printf("Uptime: %luh%lum%lus", uptime / 60 / 60, uptime / 60 % 60, uptime % 60);
}

static void cli_command_log_tx_callback(const uint8_t* buffer, size_t size, void* context) {
    PipeSide* pipe = context;
    pipe_send(pipe, buffer, size);
}

static bool cli_command_log_level_set_from_string(FuriString* level) {
    FuriLogLevel log_level;
    if(furi_log_level_from_string(furi_string_get_cstr(level), &log_level)) {
        furi_log_set_level(log_level);
        return true;
    } else {
        printf("<log> — start logging using the current level from the system settings\r\n");
        printf("<log error> — only critical errors and other important messages\r\n");
        printf("<log warn> — non-critical errors and warnings including <log error>\r\n");
        printf("<log info> — non-critical information including <log warn>\r\n");
        printf("<log default> — the default system log level (equivalent to <log info>)\r\n");
        printf(
            "<log debug> — debug information including <log info> (may impact system performance)\r\n");
        printf(
            "<log trace> — system traces including <log debug> (may impact system performance)\r\n");
    }
    return false;
}

void cli_command_log(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    FuriLogLevel previous_level = furi_log_get_level();
    bool restore_log_level = false;

    if(furi_string_size(args) > 0) {
        if(!cli_command_log_level_set_from_string(args)) {
            return;
        }
        restore_log_level = true;
    }

    const char* current_level;
    furi_log_level_to_string(furi_log_get_level(), &current_level);
    printf("Current log level: %s\r\n", current_level);

    FuriLogHandler log_handler = {
        .callback = cli_command_log_tx_callback,
        .context = pipe,
    };

    furi_log_add_handler(log_handler);

    printf("Use <log ?> to list available log levels\r\n");
    printf("Press CTRL+C to stop...\r\n");
    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        furi_delay_ms(100);
    }

    furi_log_remove_handler(log_handler);

    if(restore_log_level) {
        // There will be strange behaviour if log level is set from settings while log command is running
        furi_log_set_level(previous_level);
    }
}

void cli_command_top(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    int interval = 1000;
    args_read_int_and_trim(args, &interval);
    
    printf(ANSI_ERASE_DISPLAY(ANSI_ERASE_ENTIRE)); // Clear display
    
    FuriThreadList* thread_list = furi_thread_list_alloc();
    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
        uint32_t tick = furi_get_tick();
        furi_thread_enumerate(thread_list);

        if(interval) printf(ANSI_CURSOR_POS("1", "1"));

        uint32_t uptime = tick / furi_kernel_get_tick_frequency();
        printf(
            "Threads: %zu, ISR Time: %0.2f%%, Uptime: %luh%lum%lus" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            furi_thread_list_size(thread_list),
            (double)furi_thread_list_get_isr_time(thread_list),
            uptime / 60 / 60,
            uptime / 60 % 60,
            uptime % 60);

        printf(
            "Heap: total %zu, free %zu, minimum %zu, max block %zu" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n" ANSI_ERASE_LINE(ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            memmgr_get_total_heap(),
            memmgr_get_free_heap(),
            memmgr_get_minimum_free_heap(),
            memmgr_heap_get_max_free_block());

        printf(
            "%-25s %-20s %-10s %5s %12s %6s %10s %7s %5s" ANSI_ERASE_LINE(
                ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
            "AppID",
            "Name",
            "State",
            "Prio",
            "Stack start",
            "Stack",
            "Stack Min",
            "Heap",
            "%CPU");

        for(size_t i = 0; i < furi_thread_list_size(thread_list); i++) {
            const FuriThreadListItem* item = furi_thread_list_get_at(thread_list, i);
            printf(
                "%-25s %-20s %-10s %5d   0x%08lx %6lu %10lu %7zu %5.1f" ANSI_ERASE_LINE(
                    ANSI_ERASE_FROM_CURSOR_TO_END) "\r\n",
                item->app_id,
                item->name,
                item->state,
                item->priority,
                item->stack_address,
                item->stack_size,
                item->stack_min_free,
                item->heap,
                (double)item->cpu);
        }

        printf(ANSI_ERASE_DISPLAY(ANSI_ERASE_FROM_CURSOR_TO_END));
        fflush(stdout);

        if(interval > 0) {
            furi_delay_ms(interval);
        } else {
            break;
        }
    }
    furi_thread_list_free(thread_list);
}


void cli_command_free(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    printf("Free heap size: %zu\r\n", memmgr_get_free_heap());
    printf("Total heap size: %zu\r\n", memmgr_get_total_heap());
    printf("Minimum heap size: %zu\r\n", memmgr_get_minimum_free_heap());
    printf("Maximum heap block: %zu\r\n", memmgr_heap_get_max_free_block());

    printf("Pool free: %zu\r\n", memmgr_pool_get_free());
    printf("Maximum pool block: %zu\r\n", memmgr_pool_get_max_block());
}

void cli_command_free_blocks(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    memmgr_heap_printf_free_blocks();
}

FuriHalClockSource cli_clock_sources[] = {
    FuriHalClockSourceNone,
    FuriHalClockSourcePllSys,
    FuriHalClockSourcePllUsb,
    FuriHalClockSourcePllUsbPrimaryRefOpcg,
    FuriHalClockSourceRosc,
    FuriHalClockSourceXosc,
    FuriHalClockSourceLposc,
    FuriHalClockSourceSys,
    FuriHalClockSourceUsb,
    FuriHalClockSourceAdc,
    FuriHalClockSourceRef,
    FuriHalClockSourcePeri,
    FuriHalClockSourceHstx,
    FuriHalClockSourceOtp2fc,
};

static void cli_command_clock_out_help(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);
    printf(
        "Usage: Outputting clock to GPIO13 with divider <CLOCK_SOURCE> <DIV>\r\n"
        "Where <CLOCK_SOURCE> is:\r\n"
        "\tNone \t\t\t0\r\n"
        "\tPllSys \t\t\t1\r\n"
        "\tPllUsb \t\t\t2\r\n"
        "\tPllUsbPrimaryRefOpcg \t3\r\n"
        "\tRosc \t\t\t4\r\n"
        "\tXosc \t\t\t5\r\n"
        "\tLposc \t\t\t6\r\n"
        "\tSys \t\t\t7\r\n"
        "\tUsb \t\t\t8\r\n"
        "\tAdc \t\t\t9\r\n"
        "\tRef \t\t\t10\r\n"
        "\tPeri \t\t\t11\r\n"
        "\tHstx \t\t\t12\r\n"
        "\tOtp2fc \t\t\t13\r\n");
    printf(
        "Where <DIV> is: a divider for the clock, for example 1, 2, 4, 8, \r\n"
        "\tetc. The output frequency will be CLOCK_SOURCE_FREQ / DIV\r\n");
}

void cli_command_clock_out(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(furi_string_size(args) < 1) {
        cli_command_clock_out_help(pipe, args, context);
        return;
    }

    int source = 0;
    int div = 0;
    if(!args_read_int_and_trim(args, &source)) {
        cli_command_clock_out_help(pipe, args, context);
        return;
    }
    if(source < 0 || source >= sizeof(cli_clock_sources) / sizeof(FuriHalClockSource)) {
        cli_command_clock_out_help(pipe, args, context);
        return;
    }

    if(furi_string_size(args) < 1 && cli_clock_sources[source] != FuriHalClockSourceNone) {
        cli_command_clock_out_help(pipe, args, context);
        return;
    }

    if(cli_clock_sources[source] != FuriHalClockSourceNone) {
        if(!args_read_int_and_trim(args, &div)) {
            cli_command_clock_out_help(pipe, args, context);
            return;
        }
        if(div < 0) {
            cli_command_clock_out_help(pipe, args, context);
            return;
        }
        printf("Outputting clock %d to GPIO13 with divider %d\r\n", source, div);
    } else {
        printf("Disabling clock output to GPIO13\r\n");
    }

    furi_hal_clock_out_to_gpio13(cli_clock_sources[source], (float)div);
}

static void cli_command_otp_help(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);
    printf(
        "otp <OTP_ACTION>\r\n"
        "Where <OTP_ACTION> is:\r\n"
        "\twhitelabel <FIRMWARE_ID> <BODY_ID> <CONNECTIVITY_ID>\r\n"
        "");
}

void cli_command_otp(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* action = furi_string_alloc();
    do {
        if(!args_read_string_and_trim(args, action)) {
            cli_command_otp_help(pipe, args, context);
            break;
        }

        if(furi_string_cmp(action, "whitelabel") != 0) {
            cli_command_otp_help(pipe, args, context);
            break;
        }

        int firmware_id, body_id, connectivity_id;
        if(!args_read_int_and_trim(args, &firmware_id) || !args_read_int_and_trim(args, &body_id) || !args_read_int_and_trim(args, &connectivity_id)) {
            cli_command_otp_help(pipe, args, context);
            break;
        }

        if(firmware_id < 0 || body_id < 0 || connectivity_id < 0) {
            cli_command_otp_help(pipe, args, context);
            break;
        }

        printf("Programming USB white label in OTP with F%dB%dC%d\r\n", firmware_id, body_id, connectivity_id);

        if(furi_hal_otp_usb_white_label_valid()) {
            printf("USB white label is already programmed in OTP, it cannot be programmed again\r\n");
            break;
        }

        FuriHalOtpUsbWhiteLabelError error = furi_hal_otp_write_usb_white_label(firmware_id, body_id, connectivity_id);
        if(error == FuriHalOtpUsbWhiteLabelErrorNone) {
            printf("USB white label written to OTP successfully\r\n");
        } else {
            printf("Failed to write USB white label to OTP: %d\r\n", error);
        }
    } while(0);
    furi_string_free(action);
}