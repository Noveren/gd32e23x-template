#ifndef __BSP_H_
#define __BSP_H_

#include <stdint.h>

void bsp_init(void);

void bsp_pa15_led_on(void);
void bsp_pa15_led_off(void);
void bsp_pa15_led_toggle(void);

void bsp_systick_delay_await_us(uint32_t us) ;
void bsp_systick_delay_await_ms(uint32_t ms);
// void bps_uart_enable(void);
// void bps_uart_disable(void);

#endif