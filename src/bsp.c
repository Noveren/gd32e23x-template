
#include "gd32e23x.h"
#include "bsp.h"

volatile RingQ __uart_receive_ringq;
#define UART_RECEIVE_RINGQ_SIZE 512
static uint8_t uart_receive_ringq_space[UART_RECEIVE_RINGQ_SIZE];

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

    ringq_init((RingQ*)(&__uart_receive_ringq), UART_RECEIVE_RINGQ_SIZE, uart_receive_ringq_space);

    usart_receive_fifo_enable(USART0);
    usart_receiver_timeout_threshold_config(USART0, 18); // a magic number based on baudrate 115200
    nvic_irq_enable(USART0_IRQn, 3);
    usart_interrupt_disable(USART0, USART_INT_RBNE);
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

void bsp_uart_receive_enable(void) {
    usart_receiver_timeout_enable(USART0); // 接收到一个字节后再接收一个字节，才启动超时检测
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_interrupt_enable(USART0, USART_INT_RBNE);
}

void bsp_uart_receive_disable(void) {
    usart_receive_config(USART0, USART_RECEIVE_DISABLE);
    usart_receiver_timeout_disable(USART0);
    usart_interrupt_disable(USART0, USART_INT_RBNE);
}

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

uint32_t bsp_uart_receive(uint8_t out[], uint32_t max_len) {
    uint32_t counter = 0;
    usart_interrupt_disable(USART0, USART_INT_RBNE);
    while (!ringq_is_empty(&__uart_receive_ringq)) {
        if (counter >= max_len) {
            break;
        }
        ringq_poll((RingQ*)(&__uart_receive_ringq), &out[counter]);
        counter += 1;
    }
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    return counter;
}

/// FIXME: 过长输入现象非理想
uint32_t bsp_uart_receive_await(uint8_t out[], uint32_t max_len) {
    for (;;) { // wait for data
        usart_interrupt_disable(USART0, USART_INT_RBNE);
        if (!ringq_is_empty(&__uart_receive_ringq)) {
            usart_interrupt_enable(USART0, USART_INT_RBNE);
            break;
        } else {
            usart_interrupt_enable(USART0, USART_INT_RBNE);
            bsp_systick_delay_await_ms(100);
        }
    }
    return bsp_uart_receive(out, max_len);
}