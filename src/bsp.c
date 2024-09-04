
#include "gd32e23x.h"
#include "bsp.h"

void bsp_init(void) {
    // bsp_pa15_init
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);

    // bsp_systick_init
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    uint32_t us_ticks = SystemCoreClock / 1000000UL;
    SysTick->LOAD = (uint32_t)(us_ticks - 1UL);
    nvic_irq_disable(SysTick_IRQn);
}

void bsp_pa15_led_on(void)     { GPIO_BC(GPIOA)  = (uint32_t)(GPIO_PIN_15); }
void bsp_pa15_led_off(void)    { GPIO_BOP(GPIOA) = (uint32_t)(GPIO_PIN_15); }
void bsp_pa15_led_toggle(void) { GPIO_TG(GPIOA)  = (uint32_t)(GPIO_PIN_15); }

void bsp_systick_delay_await_us(uint32_t us) {
    SysTick->VAL = (SysTick->LOAD + 1UL);
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    for (; us > 0; us--) {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0UL);
    }
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

#define MAX_UINT32_T 0xFFFFFFFFU
void bsp_systick_delay_await_ms(uint32_t ms) {
    while (ms > 0) {
        if (MAX_UINT32_T / 1000U < ms) {
            bsp_systick_delay_await_us(MAX_UINT32_T);
            ms -= MAX_UINT32_T / 1000U;
        } else {
            bsp_systick_delay_await_us(ms * 1000U);
            ms = 0;
        }
    }
}