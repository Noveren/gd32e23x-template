
#include "gd32e23x.h"

/// Non Maskable Interrupt
void NMI_Handler(void) {

}

void HardFault_Handler(void) {

}

/// Service Call
void SVC_Handler(void) {

}

void PendSV_Handler(void) {

}

void SysTick_Handler(void) {

}

extern int __impl_tool_io_getchar_ringq_push(const uint8_t byte); // gd32e23x_tool.c
void USART0_IRQHandler(void) {
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE_ORERR);
        // WARN: 串口响应速度需要足够快
        for (;;) {
            if (usart_flag_get(USART0, USART_FLAG_RBNE)) {
                // The overflow data will be ditched without any warning
                __impl_tool_io_getchar_ringq_push((uint8_t)(GET_BITS(USART_RDATA(USART0), 0U, 8U)));
            } else {
                break;
            }
        }
    } else {
        USART_INTC(USART0) |= (
              0b1 << 20
            | 0b1 << 17
            | 0b1 << 12
            | 0b1 << 11
            | 0b1 << 9
            | 0b1 << 8
            | 0b1 << 6
            | 0b1 << 4
            | 0b1 << 3
            | 0b1 << 2
            | 0b1 << 1
            | 0b1 << 0
        );
    }
    NVIC_ClearPendingIRQ(USART0_IRQn);
}

void RTC_IRQHandler(void) {
    if (RESET != rtc_flag_get(RTC_STAT_ALRM0F)) {
        rtc_flag_clear(RTC_STAT_ALRM0F);
        exti_flag_clear(EXTI_17);
    }
}