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

void bsp_uart_enable(void);
void bsp_uart_disable(void);
void bsp_uart_transmit_enable(void);
void bsp_uart_transmit_disable(void);
void bsp_uart_receive_enable(void);
void bsp_uart_receive_disable(void);

void bsp_uart_transmit_byte_await(const uint8_t byte);
void bsp_uart_transmit_bytes_await(const uint8_t bytes[], const uint32_t len);
uint32_t bsp_uart_transmit_cstr_await(const char cstr[]);

#ifdef DEBUG
    #define BSP_LOG(cstr) bsp_uart_transmit_cstr_await(cstr)
    #define BSP_TODO(cstr) do {                \
        bsp_uart_transmit_cstr_await("TODO "); \
        bsp_uart_transmit_cstr_await(cstr);    \
    } while (0)
#else
    #define BSP_LOG(cstr) 0
    #define BSP_TODO(cstr) do {                    \
        bsp_uart_transmit_cstr_await("NOT IMPL "); \
        bsp_uart_transmit_cstr_await(cstr);        \
    } while (0)
#endif

#include "ringq.h"
extern volatile RingQ __uart_receive_ringq;

uint32_t bsp_uart_receive(uint8_t out[], uint32_t max_len);
uint32_t bsp_uart_receive_await(uint8_t out[], uint32_t max_len);

#endif