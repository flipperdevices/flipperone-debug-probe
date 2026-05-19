#include "cli_command_gpio.h"

#include <furi_hal_resources.h>
#include <args.h>

typedef struct {
    const char* name;
    const GpioPin* pin;
    const GpioMode mode;
} CliGpioPinNamePair;

static bool cli_command_gpio_parse_value(FuriString* args, uint8_t* value) {
    bool ret = false;

    int pin_mode; //-V779
    if(args_read_int_and_trim(args, &pin_mode) && (pin_mode == 0 || pin_mode == 1)) {
        ret = true;
        *value = pin_mode;
    }

    return ret;
}

static const CliGpioPinNamePair gpios[] = {
    {
        .name = "mcu_m40",
        .pin = &gpio_mcu_m40,
        .mode = GpioModeInput,
    },
    // {
    //     .name = "mcu_m41",
    //     .pin = &gpio_mcu_m41,
    //     .mode = GpioModeInput,
    // },
    {
        .name = "cpu_d2",
        .pin = &gpio_cpu_d2,
        .mode = GpioModeInput,
    },
    {
        .name = "cpu_d3",
        .pin = &gpio_cpu_d3,
        .mode = GpioModeInput,
    },
    {
        .name = "mcu_reset",
        .pin = &gpio_mcu_reset,
        .mode = GpioModeOutputOpenDrain,
    },
    {
        .name = "cpu_reset",
        .pin = &gpio_cpu_reset,
        .mode = GpioModeOutputOpenDrain,
    },

};

static void gpio_print_pins(void) {
    uint8_t n = COUNT_OF(gpios);
    for(uint8_t i = 0; i < n; i++) {
        if(gpios[i].mode == GpioModeInput) {
            printf("\t%s\t\t(input mode)\r\n", gpios[i].name);
        } else if(gpios[i].mode == GpioModeOutputOpenDrain) {
            printf("\t%s\t(output open-drain mode)\r\n", gpios[i].name);
        } else if(gpios[i].mode == GpioModeOutputPushPull) {
            printf("\t%s\t(output push-pull mode)\r\n", gpios[i].name);
        } else {
            printf("\t%s\t(unsupported %u mode )\r\n", gpios[i].name, gpios[i].mode);
        }
    }
}

static void cli_command_gpio_print_usage(void) {
    printf("Usage:\r\n");
    printf("gpio <pin_name> <0|1>\t - Set gpio value, if pin is output\r\n");
    printf("gpio <pin_name>\t\t - Read gpio value, if pin is input\r\n");
    printf("Pins: ");
    gpio_print_pins();
}

static bool cli_command_gpio_parse_pin(FuriString* args, const CliGpioPinNamePair** pin) {
    bool result = false;
    FuriString* pin_name = furi_string_alloc();
    do {
        if(!args_read_string_and_trim(args, pin_name)) break;

        for(uint8_t i = 0; i < COUNT_OF(gpios); i++) {
            if(!furi_string_equal_str(pin_name, gpios[i].name)) continue;
            *pin = &gpios[i];
            result = true;
            break;
        }
    } while(false);
    furi_string_free(pin_name);

    return result;
}

void cli_command_gpio(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);

    do {
        const CliGpioPinNamePair* pin_description = NULL;
        if(!cli_command_gpio_parse_pin(args, &pin_description)) {
            cli_command_gpio_print_usage();
            break;
        }

        uint8_t value;
        if(!cli_command_gpio_parse_value(args, &value)) {
            if(pin_description->mode == GpioModeInput) {
                bool pin_value = furi_hal_gpio_read(pin_description->pin);
                printf("Pin %s => %u\r\n", pin_description->name, pin_value);
            } else {
                printf("Pin %s is an output pin, value argument is required\r\n", pin_description->name);
                cli_command_gpio_print_usage();
            }
            break;
        }

        if(pin_description->mode == GpioModeOutputOpenDrain) {
            furi_hal_gpio_write_open_drain(pin_description->pin, value);
            printf("Pin %s => %u\r\n", pin_description->name, value);
        } else if(pin_description->mode == GpioModeOutputPushPull) {
            furi_hal_gpio_write(pin_description->pin, value);
            printf("Pin %s => %u\r\n", pin_description->name, value);
        } else if(pin_description->mode == GpioModeInput) {
            printf("Pin %s is an input pin, cannot set value\r\n", pin_description->name);
        } else {
            printf("Pin %s has unsupported mode %u\r\n", pin_description->name, pin_description->mode);
        }

    } while(false);
}
