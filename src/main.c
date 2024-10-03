
#include "app.h"

static CommandFn COMMAND_FN_TABLE[] = {
    ///
    app_cmd_get_fram_id,
    ///
    app_cmd_get_fram_status_register,
    ///
    app_cmd_get_fram_data,
    ///
    app_cmd_set_fram_write_enable,
    ///
    app_cmd_set_fram_clean,
    ///
    app_cmd_get_adc_once,
    ///
    app_cmd_set_adc_timer_start,
    ///
    app_cmd_collect_signal,
};

static int8_t parse_command(const char* input) {
    if (0 == cstrcmp(input, "<!get_fram_id>"))
        return 0;
    if (0 == cstrcmp(input, "<!get_fram_status_register>"))
        return 1;
    if (0 == cstrcmp(input, "<!get_fram_data>"))
        return 2;
    if (0 == cstrcmp(input, "<!set_fram_write_enable>"))
        return 3;
    if (0 == cstrcmp(input, "<!set_fram_clean>"))
        return 4;
    if (0 == cstrcmp(input, "<!get_adc_once>"))
        return 5;
    if (0 == cstrcmp(input, "<!set_adc_timer_start>"))
        return 6;
    if (0 == cstrcmp(input, "<!collect_signal>"))
        return 7;
    return -1;
}

void main() {
    app_init();

    app_delay_ms(2000);
    app_led_on();

    const char* input = NULL;

    app_print("Wait for input within 60 seconds, otherwise the default task will be executed.");
    if ((input = app_gets_within_x_sec_or_NULL(60)) == NULL) {
        app_log_debug("No input and wait for RTC alarm.");
        app_led_off();
        app_deepsleep_with_rtc(0x0, 0x1, 0x0);
        app_led_on();
        app_log_debug("RTC alarm");
        for (;;) {
            app_delay_ms(1000);
            app_led_toggle();
        }
    } else {
        int8_t cmd_idx = -1;
        do {
            if ((cmd_idx = parse_command(input)) < 0) {
                app_print("Invaild Command.");
                app_print(input);
            } else {
                COMMAND_FN_TABLE[cmd_idx]() ? app_print("SUCCESS") : app_print("ERROR");
            }
            input = app_gets();
        } while (1);
    }
}