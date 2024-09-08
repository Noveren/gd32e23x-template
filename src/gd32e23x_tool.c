
#include "gd32e23x_tool.h"
#include "ringq.h"

static RingQ __impl_tool_io_getchar_ringq;
static uint8_t __impl_tool_io_getchar_ringq_space[__impl_tool_io_getchar_RINGQ_SIZE];

void __impl_tool_init(void) {
    /* systick init */
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    uint32_t us_ticks = SystemCoreClock / 1000000UL;
    SysTick->LOAD = (uint32_t)(us_ticks - 1UL);
    nvic_irq_disable(SysTick_IRQn);

    /* usart init */
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

    ringq_init(
        &__impl_tool_io_getchar_ringq,
        __impl_tool_io_getchar_RINGQ_SIZE,
        __impl_tool_io_getchar_ringq_space
    );

    // usart_receiver_timeout_threshold_config(USART0, 18); // a magic number based on baudrate 115200
    nvic_irq_enable(USART0_IRQn, 3);
    usart_interrupt_disable(USART0, USART_INT_RBNE); // USART0 只使用 USART_INT_RBNE
}

void __impl_tool_delay_us(uint32_t us) {
    SysTick->VAL = (SysTick->LOAD + 1UL);
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    for (; us > 0; us--) {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0UL);
    }
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void __impl_tool_io_enable(void) {
    /// usart enable
    USART_CTL0(USART0) |= USART_CTL0_UEN;
    /// usart transmit enable
    USART_CTL0(USART0) |= USART_CTL0_TEN;
    /// usart receive enable
    usart_receive_fifo_enable(USART0);
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);

    // usart_receiver_timeout_enable(USART0); // 接收到一个字节后再接收一个字节，才启动超时检测
}

void __impl_tool_io_disable(void) {
    /// usart disable
    USART_CTL0(USART0) &= ~(USART_CTL0_UEN);
    /// usart transmit disable
    USART_CTL0(USART0) &= ~USART_CTL0_TEN;
    /// usart receive disable
    usart_receive_config(USART0, USART_RECEIVE_DISABLE);
    usart_interrupt_disable(USART0, USART_INT_RBNE);
    usart_receive_fifo_disable(USART0);

    // usart_receiver_timeout_disable(USART0);
}

void __impl_tool_io_putbyte(const uint8_t byte) {
    while (RESET == (USART_REG_VAL(USART0, USART_FLAG_TBE) & BIT(USART_BIT_POS(USART_FLAG_TBE))));
    USART_TDATA(USART0) = (uint32_t)(byte);
}

int __impl_tool_io_getchar_ringq_push(const uint8_t byte) {
    return ringq_push(&__impl_tool_io_getchar_ringq, byte);
}

int __impl_tool_io_getchar_now(void) {
    usart_interrupt_disable(USART0, USART_INT_RBNE);
    int ret = ringq_poll(&__impl_tool_io_getchar_ringq);
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    return (ret < 0) ? -1 : ret;
}