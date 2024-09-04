
// TODO 协议帧头

#include "gd32e23x.h"
#include "bsp.h"

#define RECEIVE_BUF_MAX_LEN 128

void main() {
    bsp_init();
    bsp_uart_enable();
    bsp_uart_transmit_enable();

    bsp_pa15_led_on();

    bsp_uart_receive_enable();
    uint8_t receive_buf[RECEIVE_BUF_MAX_LEN] = { 0 };
    uint32_t receive_len = 0;

    BSP_LOG("Wait for command (1 min).\n");
    for (uint32_t n_100ms = 10 * 60; n_100ms > 0; n_100ms--) {
        receive_len = bsp_uart_receive(receive_buf, RECEIVE_BUF_MAX_LEN);
        if (receive_len == 0) {
            bsp_systick_delay_await_ms(100);
        } else {
            break;
        }
    }
    if (receive_len == 0) {
        bsp_pa15_led_off();
        BSP_LOG("Sleep (1 min)\n");
        for (uint32_t min = 1; min > 0; min--) {
            bsp_systick_delay_await_ms(1000 * 60);
        }
        bsp_pa15_led_on();

        BSP_TODO("Execute default task.\n");
        
        BSP_TODO("Turn off.\n");
        while (1);
    } else {
        do {
            BSP_TODO("Parse command: ");
            bsp_uart_transmit_bytes_await(receive_buf, receive_len);
            bsp_uart_transmit_byte_await('\n');
            if (receive_len == 128) {
                BSP_LOG("Out of receive buffer.\n");
            }

            BSP_TODO("Execute command or turn off.\n");

            receive_len = bsp_uart_receive_await(receive_buf, RECEIVE_BUF_MAX_LEN);
            if (receive_len == 128) {
                BSP_LOG("Out of receive buffer.\n");
            }
        } while (1);
    }
}