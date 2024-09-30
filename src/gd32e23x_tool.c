
#include "gd32e23x_tool.h"
#include "ringq.h"

static RingQ __impl_tool_io_getchar_ringq;
static uint8_t __impl_tool_io_getchar_ringq_space[__impl_tool_io_getchar_RINGQ_SIZE];

void __impl_tool_delay_init(void) {
    /* systick init */
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    uint32_t us_ticks = SystemCoreClock / 1000000UL;
    SysTick->LOAD = (uint32_t)(us_ticks - 1UL);
    nvic_irq_disable(SysTick_IRQn);
}

void __impl_tool_io_init(void) {
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

/// 参数均采用 BCD 格式, 采用 24 小时制, 赋值范围位 0x0-0x23, 0x0-0x59, 0x0-0x59
void __impl_tool_deepsleep_with_rtc(uint8_t hour, uint8_t minute, uint8_t second) {
    // CK_RTC 时钟源由备份域控制寄存器 RCU_BDCTL 的 RTCSRC[1:0] 控制,
    // 支持 HXTAL(11), LXTAL（01), IRC40K(10), 复位值为 00,
    // 该寄存器的部分位只有在电源控制器 PMU_CTL 中 BKPWEN 置位后才能改动
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
    rcu_osci_on(RCU_IRC40K);
    rcu_osci_stab_wait(RCU_IRC40K);
    rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();

    rtc_parameter_struct rtc_init_param = {
        .rtc_am_pm = RTC_AM,
        .rtc_display_format = RTC_24HOUR,
        .rtc_factor_asyn = 0x63,    // 异步分频值, 与时钟源有关
        .rtc_factor_syn = 0x18F,    // 同步分频值, 与时钟源有关
        // 当前时间，均采用 BCD 码
        .rtc_year = 0x24,
        .rtc_month = RTC_JAN,
        .rtc_date = 0x01,
        .rtc_day_of_week = RTC_MONDAY,
        .rtc_hour = 0x0,
        .rtc_minute = 0x0,
        .rtc_second = 0x0,
    };
    if (rtc_init(&rtc_init_param) == SUCCESS) {
        rtc_alarm_struct rtc_alarm_init_param = {
            .rtc_am_pm = RTC_AM,
            .rtc_weekday_or_date = RTC_ALARM_DATE_SELECTED,
            .rtc_alarm_day = 0x01,
            .rtc_alarm_mask =
                RTC_ALARM_DATE_MASK
            ,
            // 闹钟时间, 均采用 BCD 码
            .rtc_alarm_hour = hour,
            .rtc_alarm_minute = minute,
            .rtc_alarm_second = second,
        };
        rtc_alarm_config(&rtc_alarm_init_param);

        nvic_irq_enable(RTC_IRQn, 0);
        rtc_interrupt_enable(RTC_INT_ALARM);
        exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_RISING);
        rtc_alarm_enable();

        pmu_to_deepsleepmode(PMU_LDO_NORMAL, WFI_CMD);
        SystemInit();

        rtc_alarm_disable();
        rtc_interrupt_disable(RTC_INT_ALARM);
        nvic_irq_disable(RTC_IRQn);
    }

    rcu_osci_off(RCU_IRC40K);
    rcu_periph_clock_disable(RCU_RTC);
    pmu_backup_write_disable();
    rcu_periph_clock_disable(RCU_PMU);
}

void __impl_tool_spi_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_4);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, 
        GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
    );

    spi_i2s_deinit(SPI0);
    rcu_periph_clock_enable(RCU_SPI0);
    spi_parameter_struct spi0_init = {
        .device_mode          = SPI_MASTER,
        .trans_mode           = SPI_TRANSMODE_FULLDUPLEX,
        .frame_size           = SPI_FRAMESIZE_8BIT,
        .nss                  = SPI_NSS_SOFT,
        .endian               = SPI_ENDIAN_MSB,
        .clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE,
        .prescale             = SPI_PSC_4,
    };
    spi_init(SPI0, &spi0_init);
    spi_nss_output_enable(SPI0);
}

void __impl_tool_spi_deinit(void) {
    // TODO 需要怎么处理引脚
    rcu_periph_clock_enable(RCU_SPI0);
    spi_i2s_deinit(SPI0);
    rcu_periph_clock_disable(RCU_SPI0);
}

void __impl_tool_spi_enable(void) {
    SPI_CTL0(SPI0) |= (uint32_t)( SPI_CTL0_SPIEN);
}

void __impl_tool_spi_disable(void) {
    SPI_CTL0(SPI0) &= (uint32_t)(~SPI_CTL0_SPIEN);
}

void __impl_tool_spi_select(void) {
    SPI_CTL0(SPI0) &= (uint32_t)(~SPI_CTL0_SWNSS);
}

void __impl_tool_spi_release(void) {
    SPI_CTL0(SPI0) |= (uint32_t)( SPI_CTL0_SWNSS);
}


uint8_t __impl_tool_spi_access_data(uint8_t byte) {
    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
    SPI_DATA(SPI0) = ((uint32_t)(byte));
    while(!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_RBNE)));
    return (uint8_t)SPI_DATA(SPI0);
}