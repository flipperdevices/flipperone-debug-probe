#include <cli/cli_main_shell.h>
#include <cli/cli_ansi.h>
#include <version/version.h>

void cli_main_motd(void* context) {
    UNUSED(context);
        printf(
        "\r\n"
        " ___      _                ___         _            ___ _    ___ \r\n"
        "|   \\ ___| |__ _  _ __ _  | _ \\_ _ ___| |__  ___   / __| |  |_ _|\r\n"
        "| |) / -_) '_ \\ || / _` | |  _/ '_/ _ \\ '_ \\/ -_) | (__| |__ | | \r\n"
        "|___/\\___|_.__/\\_,_\\__, | |_| |_| \\___/_.__/\\___|  \\___|____|___|\r\n"
        "                   |___/                                         \r\n"
        "\r\n"
        "Welcome to Flipper One Debug Probe Command Line Interface!\r\n"
        "   Port 1: CPU tty (1500000)\r\n"
        "   Port 2: MCU CLI (1500000)\r\n"
        "   Port 3: MCU log (1500000)\r\n"
        "   Port 4: CLI     (this interface)\r\n"
        "Read the manual: https://docs.flipper.net/development/cli\r\n"
        "Run `help` or `?` to list available commands\r\n"
        "\r\n");

    const Version* firmware_version = version_get();
    if(firmware_version) {
        printf(
            "Firmware version: %s %s (%s%s built on %s)\r\n",
            version_get_gitbranch(firmware_version),
            version_get_version(firmware_version),
            version_get_githash(firmware_version),
            version_get_dirty_flag(firmware_version) ? "-dirty" : "",
            version_get_builddate(firmware_version));
    }
}
