
#include "app.h"

typedef bool(*CommandFn)(void);
static CommandFn COMMAND_FN_TABLE[] = {

};

static int8_t parse_command(const char* cstr) {
    return -1;
}

void main() {
    app_init();

    app_delay_ms(2000);
    app_led_on();

    const char* input = NULL;

    app_print("Wait for input within 20 seconds, otherwise the default task will be executed.");
    if ((input = app_gets_within_x_sec_or_NULL(20)) == NULL) {
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