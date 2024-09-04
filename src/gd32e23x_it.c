
#include "gd32e23x.h"
#include "bsp.h"


void NMI_Handler(void) {

}

void HardFault_Handler(void) {

}

void SVC_Handler(void) {

}

void PendSV_Handler(void) {

}

void SysTick_Handler(void) {

}

void USART0_IRQHandler(void) {
    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE_ORERR);
        uint32_t counter = 0;
        for (;;) {
            if (usart_flag_get(USART0, USART_FLAG_RT)) {
                __BSP_UART_RECEIVE_BUF[counter] = '\0';
                break;
            }
            if (usart_flag_get(USART0, USART_FLAG_RBNE)) {
                __BSP_UART_RECEIVE_BUF[counter] = (uint8_t)(GET_BITS(USART_RDATA(USART0), 0U, 8U));
                counter += 1;
                if (counter == BSP_UART_RECEIVE_BUF_LENGTH) {
                    break;
                }
            }
        }
        usart_flag_clear(USART0, USART_FLAG_RT);
        __BSP_UART_RECEIVE_COUNTER = counter;
        __BSP_UART_RECEIVE_COMPLETE = true;
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