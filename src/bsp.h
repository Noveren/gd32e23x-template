#ifndef __BSP_H_
#define __BSP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void bsp_init(void);

void bsp_pa15_led_on(void);
void bsp_pa15_led_off(void);
void bsp_pa15_led_toggle(void);

void bsp_systick_delay_await_us(uint32_t us) ;
void bsp_systick_delay_await_ms(uint32_t ms);

/// 接收缓冲区字节数量，实际可用数量少 1 个 ('\0x00')
#define BSP_UART_RECEIVE_BUF_LENGTH 4 * 128

extern volatile bool __BSP_UART_RECEIVE_COMPLETE;
extern volatile uint32_t __BSP_UART_RECEIVE_COUNTER;
extern uint8_t __BSP_UART_RECEIVE_BUF[BSP_UART_RECEIVE_BUF_LENGTH];

void bsp_uart_enable(void);
void bsp_uart_disable(void);
void bsp_uart_transmit_enable(void);
void bsp_uart_transmit_disable(void);
void bsp_uart_receive_enable(void);
void bsp_uart_receive_disable(void);
void bsp_uart_transmit_byte_await(const uint8_t byte);
void bsp_uart_transmit_bytes_await(const uint8_t bytes[], const uint32_t len);
uint32_t bsp_uart_transmit_cstr_await(const char cstr[]);

const uint8_t* bsp_uart_receive_await(void);
uint32_t bsp_uart_get_last_receive_len(void);


#endif