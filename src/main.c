
// TODO 协议帧头

#include "gd32e23x.h"
#include "gd32e23x_tool.h"

#define RECEIVE_BUF_MAX_LEN 128

void app_init(void) {
    /* pa15 init */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);
}

#define app_led_on()     do { GPIO_BC(GPIOA)  = (uint32_t)(GPIO_PIN_15); } while (0)
#define app_led_off()    do { GPIO_BOP(GPIOA) = (uint32_t)(GPIO_PIN_15); } while (0)
#define app_led_toggle() do { GPIO_TG(GPIOA)  = (uint32_t)(GPIO_PIN_15); } while (0)

void main() {
    tool_init();
    app_init();

    tool_io_enable();

    tool_delay_ms(2000);
    app_led_on();

    char buf[128] = { 0 };
    uint16_t len = 0;

    tool_io_puts("Waiting fo input in x min.\n");
    uint32_t min = 1;
    for (uint32_t n100ms = min * 600; n100ms > 0; n100ms--) {
        len = tool_io_gets_now(buf, 128);
        if (len == 0) {
            tool_delay_ms(100);
        } else {
            break;
        }
    }
    if (len == 0) {
        tool_io_puts("No input.\n");
    } else {
        tool_io_puts("Get input - ");
        tool_io_puts(buf);
        tool_io_putchar('\n');
    }
    
    for (;;) {
        tool_io_puts("Waiting for input: ");
        tool_io_gets(buf, 128);
        tool_io_puts(buf);
        tool_io_putchar('\n');
        app_led_toggle();
    }
}