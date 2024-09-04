
#include "gd32e23x.h"
#include "bsp.h"

volatile bool __BSP_UART_RECEIVE_COMPLETE = false;
volatile uint32_t __BSP_UART_RECEIVE_COUNTER = 0;
uint8_t __BSP_UART_RECEIVE_BUF[BSP_UART_RECEIVE_BUF_LENGTH] = { 0 };

/// 1. 灯光控制：占有外设 PA15；
/// 2. 阻塞延时：占有外设 Systick；
/// 3. 串口使用：占有外设 USART0，PB6(TX)，PB7(RX)；8 数据位、1 停止位、波特率 115200、无校验
void bsp_init(void) {
    /* bsp_pa15_init */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);

    /* bsp_systick_init */
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    uint32_t us_ticks = SystemCoreClock / 1000000UL;
    SysTick->LOAD = (uint32_t)(us_ticks - 1UL);
    nvic_irq_disable(SysTick_IRQn);

    /* bsp_usart_init */
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_af_set(GPIOB, GPIO_AF_0, GPIO_PIN_6 | GPIO_PIN_7);

    usart_deinit(USART0);
    rcu_periph_clock_enable(RCU_USART0);
    rcu_usart_clock_config(RCU_USART0SRC_CKAPB2);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_oversample_config(USART0, USART_OVSMOD_8);
    usart_baudrate_set(USART0, 115200);
    usart_receive_fifo_enable(USART0);
    usart_receiver_timeout_threshold_config(USART0, 18);
    nvic_irq_enable(USART0_IRQn, 3);
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

void bsp_uart_enable(void)           { USART_CTL0(USART0) |= USART_CTL0_UEN;    }
void bsp_uart_disable(void)          { USART_CTL0(USART0) &= ~(USART_CTL0_UEN); }
void bsp_uart_transmit_enable(void)  { USART_CTL0(USART0) |= USART_CTL0_TEN;    }
void bsp_uart_transmit_disable(void) { USART_CTL0(USART0) &= ~USART_CTL0_TEN;   }
void bsp_uart_receive_enable(void)   { USART_CTL0(USART0) |= USART_CTL0_REN;    }
void bsp_uart_receive_disable(void)  { USART_CTL0(USART0) &= ~USART_CTL0_REN;   }

/// 发送一个字节
void bsp_uart_transmit_byte_await(const uint8_t byte) {
    while (RESET == (USART_REG_VAL(USART0, USART_FLAG_TBE) & BIT(USART_BIT_POS(USART_FLAG_TBE))));
    USART_TDATA(USART0) = (uint32_t)(byte);
}

/// 发送指定数量的字节
void bsp_uart_transmit_bytes_await(const uint8_t bytes[], const uint32_t len) {
    for (int i = 0; i < len; i++) {
        while (RESET == (USART_REG_VAL(USART0, USART_FLAG_TC) & BIT(USART_BIT_POS(USART_FLAG_TC))));
        USART_TDATA(USART0) = (uint32_t)(bytes[i]);
    }
}

/// 发送以 `\x00` 结尾的 `char` 序列
uint32_t bsp_uart_transmit_cstr_await(const char cstr[]) {
    uint32_t counter = 0;
    while (cstr[counter] != '\0') {
        while (RESET == (USART_REG_VAL(USART0, USART_FLAG_TC) & BIT(USART_BIT_POS(USART_FLAG_TC))));
        USART_TDATA(USART0) = (uint32_t)(cstr[counter++]);
    }
    return counter;
}

/// 接收串口数据（至少 2 字节）; 依赖中断
/// 接收的串口数据保存在可用长度为 `UART_RECEIVE_BUF_LENGTH-1` 的缓冲区中, 数据末尾将自动添加 `\x00`;
/// 若接收数据的长度超出缓冲区可用长度，则中止接收并返回 `NULL`
const uint8_t* bsp_uart_receive_await(void) {
    __BSP_UART_RECEIVE_COUNTER = 0;
    __BSP_UART_RECEIVE_COMPLETE = false;
    usart_receiver_timeout_enable(USART0); // 接收到一个字节后再接收一个字节，才启动超时检测
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    while (!__BSP_UART_RECEIVE_COMPLETE);
    usart_interrupt_disable(USART0, USART_INT_RBNE);
    usart_receiver_timeout_disable(USART0);
    return __BSP_UART_RECEIVE_COUNTER == BSP_UART_RECEIVE_BUF_LENGTH ? NULL : __BSP_UART_RECEIVE_BUF;
}

/// 获得上一次接收到的串口数据的字节数量
uint32_t bsp_uart_get_last_receive_len(void)  {
    return __BSP_UART_RECEIVE_COUNTER;
}
