
#include "app.h"
#include "driver.h"

void app_init(void) {
    /* pa15 init */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);

    dvr_init();
    dvr_io_get_enable();
}

void app_deinit(void) {
    dvr_io_get_disable();
    dvr_deinit();
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
}

static char app_input_buf[APP_INPUT_BUF_SIZE] = { 0 };

void app_delay_ms(uint32_t ms) {
    dvr_delay_ms(ms);
}

const char* app_gets(void) {
    return dvr_io_gets(app_input_buf, APP_INPUT_BUF_SIZE);
}

const char* app_gets_within_x_sec_or_NULL(uint8_t sec) {
    return dvr_io_gets_within_x_sec_or_NULL(app_input_buf, APP_INPUT_BUF_SIZE, sec);
}

void app_deepsleep_with_rtc(uint8_t hour, uint8_t minute, uint8_t second) {
    dvr_deepsleep_with_rtc(hour, minute, second);
}

void app_putframe_header(char type, uint32_t len) {
    dvr_io_putchar(app_FRAME_PREFIX);
    dvr_io_putchar(type);
    dvr_io_putchar('0');
    dvr_io_putchar('x');
    dvr_io_putbytes_text_reverse((uint8_t*)(&len), sizeof(uint32_t)/sizeof(uint8_t), '\x00');
    dvr_io_putchar(' ');
}

void app_putframe_footer(void) {
    dvr_io_putchar(app_FRAME_SUFFIX_0);
    dvr_io_putchar(app_FRAME_SUFFIX_1);
}

void app_print(const char* cstr) {
    app_putframe_header(app_FRAME_TYPE_TEXT, cstrlen(cstr));
    dvr_io_puts(cstr);
    app_putframe_footer();
}

void app_log_debug(const char* cstr) {
    const char* s = "DEBUG - ";
    app_putframe_header(app_FRAME_TYPE_TEXT, cstrlen(s)+cstrlen(cstr));
    dvr_io_puts(s);
    dvr_io_puts(cstr);
    app_putframe_footer();
}