
#include "gd32e23x.h"
#include "bsp.h"

void main() {
    bsp_init();
    bsp_uart_enable();
    bsp_uart_transmit_enable();

    bsp_pa15_led_off();
    bsp_systick_delay_await_ms(2000);
    bsp_pa15_led_toggle();


    bsp_uart_receive_enable();
    const uint8_t* buf = NULL;
    for (;;) {
        buf = bsp_uart_receive_await();
        if (buf) {
            bsp_uart_transmit_bytes_await(buf, bsp_uart_get_last_receive_len());
        }
    }
}