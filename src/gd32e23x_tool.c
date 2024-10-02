
#include "gd32e23x_tool.h"
#include "ringq.h"

static RingQ __impl_tool_io_getchar_ringq;
static uint8_t __impl_tool_io_getchar_ringq_space[__impl_tool_io_getchar_RINGQ_SIZE];

void __impl_tool_led_init(void) {
    /* pa15 init */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);
}

/// TODO 考虑是否可以在 gd32e23x_tool.h 中添加宏函数实现功能
void __impl_tool_led_on(void)     { GPIO_BC(GPIOA)  = (uint32_t)(GPIO_PIN_15); }
void __impl_tool_led_off(void)    { GPIO_BOP(GPIOA) = (uint32_t)(GPIO_PIN_15); }
void __impl_tool_led_toggle(void) { GPIO_TG(GPIOA)  = (uint32_t)(GPIO_PIN_15); }
// #define app_led_on()     do { GPIO_BC(GPIOA)  = (uint32_t)(GPIO_PIN_15); } while (0)
// #define app_led_off()    do { GPIO_BOP(GPIOA) = (uint32_t)(GPIO_PIN_15); } while (0)
// #define app_led_toggle() do { GPIO_TG(GPIOA)  = (uint32_t)(GPIO_PIN_15); } while (0)

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

    usart_receiver_timeout_threshold_config(USART0, 18); // a magic number based on baudrate 115200
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

    usart_receiver_timeout_enable(USART0); // FIXME: 接收到一个字节后再接收一个字节，才启动超时检测
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

    usart_receiver_timeout_disable(USART0);
}

void __impl_tool_io_putbyte(const uint8_t byte) {
    while (RESET == (USART_REG_VAL(USART0, USART_FLAG_TBE) & BIT(USART_BIT_POS(USART_FLAG_TBE))));
    USART_TDATA(USART0) = (uint32_t)(byte);
}

int __impl_tool_io_getchar_ringq_push(const uint8_t byte) {
    return ringq_push(&__impl_tool_io_getchar_ringq, byte);
}

bool __impl_tool_io_getchar_is_empty(void) {
    usart_interrupt_disable(USART0, USART_INT_RBNE);
    bool ret = ringq_is_empty(&__impl_tool_io_getchar_ringq);
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    return ret;
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

static uint16_t __impl_tool_adc_BUF[4] = { 0 };
void __impl_tool_adc_init(const uint8_t tool_adc_CHANNEL) {
    rcu_periph_clock_enable(RCU_ADC);

    /// 采样编码
    adc_resolution_config(ADC_RESOLUTION_12B);
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);

    /// ADC 采样时钟
    rcu_osci_on(RCU_IRC28M);
    rcu_osci_stab_wait(RCU_IRC28M);
    rcu_adc_clock_config(RCU_ADC_IRC28M_DIV2);

    /// 单次扫描模式
    adc_special_function_config(ADC_CONTINUOUS_MODE, DISABLE);
    adc_special_function_config(ADC_SCAN_MODE, ENABLE);
    uint8_t RSQ_idx = 0;
#define ADC_SAMPLETIME ADC_SAMPLETIME_13POINT5
    if (tool_adc_CHANNEL == 0x00) {
        adc_regular_channel_config(RSQ_idx++, ADC_CHANNEL_0, ADC_SAMPLETIME);

    } else {
        if (tool_adc_CHANNEL & tool_adc_CHANNEL_0) {
            adc_regular_channel_config(RSQ_idx++, ADC_CHANNEL_0, ADC_SAMPLETIME);
        }
        if (tool_adc_CHANNEL & tool_adc_CHANNEL_1) {
            adc_regular_channel_config(RSQ_idx++, ADC_CHANNEL_1, ADC_SAMPLETIME);
        }
        if (tool_adc_CHANNEL & tool_adc_CHANNEL_2) {
            adc_regular_channel_config(RSQ_idx++, ADC_CHANNEL_2, ADC_SAMPLETIME);
        }
        if (tool_adc_CHANNEL & tool_adc_CHANNEL_3) {
            adc_regular_channel_config(RSQ_idx++, ADC_CHANNEL_3, ADC_SAMPLETIME);
        }
    }
#undef ADC_SAMPLETIME
    adc_channel_length_config(ADC_REGULAR_CHANNEL, RSQ_idx);

    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_PULLDOWN, (uint32_t)tool_adc_CHANNEL);

    /// DMA 数据传送
    rcu_periph_clock_enable(RCU_DMA);
    dma_deinit(DMA_CH0);
    dma_parameter_struct adc_to_memory = {
        .periph_addr  = (uint32_t)(&ADC_RDATA),             /// target 为 32 位设备
        .periph_width = DMA_PERIPHERAL_WIDTH_16BIT,
        .memory_width = DMA_MEMORY_WIDTH_16BIT,
        .periph_inc   = DMA_PERIPH_INCREASE_DISABLE,
        .memory_inc   = DMA_MEMORY_INCREASE_ENABLE,
        .direction    = DMA_PERIPHERAL_TO_MEMORY,
        .number       = RSQ_idx,
        .memory_addr  = (uint32_t)(&__impl_tool_adc_BUF),
        .priority     = DMA_PRIORITY_HIGH,
    };
    dma_init(DMA_CH0, &adc_to_memory);
    dma_memory_to_memory_disable(DMA_CH0);
    dma_circulation_enable(DMA_CH0);
    // dma_flag_clear(DMA_CH0, DMA_FLAG_FTF);
    dma_channel_enable(DMA_CH0);

    /// 外部触发
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
    adc_external_trigger_source_config(ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_NONE);

    /// 启动并校准
    adc_enable();
    while (SET != (ADC_CTL1 & ADC_CTL1_ADCON));
    tool_delay_us(1000);
    adc_calibration_enable();
    adc_dma_mode_enable();
    adc_flag_clear(ADC_FLAG_STRC);
}

void __impl_tool_adc_deinit(void) {
    adc_dma_mode_disable();
    adc_disable();
    dma_channel_disable(DMA_CH0);
    /// TODO 检查 DMA 是否全部未使用，若是则关闭 DMA 时钟
    rcu_osci_off(RCU_IRC28M);
    rcu_periph_clock_disable(RCU_ADC);
}

bool __impl_tool_adc_convert_once_async(void) {
    /// adc_flag_get(ADC_FLAG_STRC) == SET
    if (ADC_STAT & ADC_FLAG_STRC) {
        return false;
    }
    /// dma_flag_clear(DMA_CH0, DMA_FLAG_FTF);
    DMA_INTC |= DMA_FLAG_ADD(DMA_FLAG_FTF, DMA_CH0);
    /// adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
    ADC_CTL1 |= ADC_CTL1_SWRCST;
    return true;
}

const uint16_t* __impl_tool_adc_get_result(void) {
    /// dma_flag_get(DMA_CH0, DMA_FLAG_FTF) == SET
    if (RESET != (DMA_INTF & DMA_FLAG_ADD(DMA_FLAG_FTF, DMA_CH0))) {
        /// adc_flag_clear(ADC_FLAG_STRC);
        ADC_STAT &= ~((uint32_t)ADC_FLAG_STRC);
        /// dma_flag_clear(DMA_CH0, DMA_FLAG_FTF);
        DMA_INTC |= DMA_FLAG_ADD(DMA_FLAG_FTF, DMA_CH0);
        return __impl_tool_adc_BUF;
    } else {
        return NULL;
    }
}

void __impl_tool_timer_init(uint32_t freq) {
    uint32_t ck_timer_clock =rcu_clock_freq_get(CK_APB2);
    if (GET_BITS(RCU_CFG0, 11, 13) >= 0b100) {
        ck_timer_clock <<= 1;
    }
    rcu_periph_clock_enable(RCU_TIMER5);
    timer_deinit(TIMER5);
    timer_prescaler_config(TIMER5, ((uint16_t)(ck_timer_clock / freq)) - 1, TIMER_PSC_RELOAD_NOW);
    nvic_irq_enable(TIMER5_IRQn, 3);
}

void __impl_tool_timer_deinit(void) {
    nvic_irq_disable(TIMER5_IRQn);
    rcu_periph_clock_disable(RCU_TIMER5);
}

CallbackFn __impl_tool_timer_timer5_callbackfn = NULL;
bool __impl_tool_timer_enable(uint16_t autoreload, CallbackFn fn) {
    if (__impl_tool_timer_timer5_callbackfn != NULL) {
        return false;
    }
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);
    timer_enable(TIMER5);
    __impl_tool_timer_timer5_callbackfn = fn;
    timer_autoreload_value_config(TIMER5, autoreload == 0 ? 0 : autoreload - 1);
    return true;
}

void __impl_tool_timer_disable(void) {
    __impl_tool_timer_timer5_callbackfn = NULL;
    timer_disable(TIMER5);
    timer_interrupt_disable(TIMER5, TIMER_INT_UP);
}

bool __impl_tool_timer_is_working(void) {
    return __impl_tool_timer_timer5_callbackfn != NULL;
}
