
#include "gd32e23x.h"
#include "util.h"

/// Non Maskable Interrupt
void NMI_Handler(void) {
    usart_data_transmit(USART0, 'N');
}

void HardFault_Handler(void) {
    usart_data_transmit(USART0, 'H');
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

extern CallbackFn __impl_dvr_timer_timer5_callbackfn;   /// TODO 此处不能加 volatile
extern void __impl_dvr_timer_disable(void);
void TIMER5_IRQHandler(void) {
    if (timer_interrupt_flag_get(TIMER5, TIMER_INT_FLAG_UP)) {
        timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);
        /// FIXME 如何防止回调函数执行时间过长
        if (!__impl_dvr_timer_timer5_callbackfn(NULL)) {
            __impl_dvr_timer_disable();
        };
    }
    NVIC_ClearPendingIRQ(TIMER5_IRQn);
}