
#include "gd32e23x.h"
#include "util.h"

/// Non Maskable Interrupt
void NMI_Handler(void) {
    const char* cstr = "NMI_Handler";
    for (uint8_t i = 0; cstr[i]; i++) {
        USART_TDATA(USART0) = (USART_TDATA_TDATA & cstr[i]);
    }
    for (;;) {}
}

void HardFault_Handler(void) {
    const char* cstr = "HardFault_Handler";
    for (uint8_t i = 0; cstr[i]; i++) {
        USART_TDATA(USART0) = (USART_TDATA_TDATA & cstr[i]);
    }
    for (;;) {}
}

/// Service Call
void SVC_Handler(void) {

}

void PendSV_Handler(void) {

}

void SysTick_Handler(void) {

}

extern int __impl_dvr_io_ringq_push(uint8_t byte); // gd32e23x_tool.c
void USART0_IRQHandler(void) {
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE_ORERR);
        for (;;) {
            if (usart_flag_get(USART0, USART_FLAG_RT)) {
                usart_flag_clear(USART0, USART_FLAG_RT);
                break;
            }
            if (usart_flag_get(USART0, USART_FLAG_RBNE)) {
                // The overflow data will be ditched without any warning
                __impl_dvr_io_ringq_push((uint8_t)(GET_BITS(USART_RDATA(USART0), 0U, 8U)));
            }
        }
    }
    NVIC_ClearPendingIRQ(USART0_IRQn);
}

void RTC_IRQHandler(void) {
    if (RESET != rtc_flag_get(RTC_STAT_ALRM0F)) {
        rtc_flag_clear(RTC_STAT_ALRM0F);
        exti_flag_clear(EXTI_17);
    }
}

extern bool dvr_timer_callback(void* _);
void TIMER5_IRQHandler(void) {
    if (timer_interrupt_flag_get(TIMER5, TIMER_INT_FLAG_UP)) {
        timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);
        /// FIXME 如何防止回调函数执行时间过长
        if (!dvr_timer_callback(NULL)) {
            timer_disable(TIMER5);
        };
    }
    NVIC_ClearPendingIRQ(TIMER5_IRQn);
}