/******************************************************************************
 * @file     startup_gd32e230k8t6.s
 * @brief    CMSIS-Core Device Startup File for Cortex-M23 Device GD32E230K8T6
 * @version  V2.2.0
 * @date     26. May 2021
 ******************************************************************************/

                .syntax  unified
                .arch    armv8-m.base
                .fpu     softvfp
                .thumb

                .global  g_pfnVectors
                .global  Default_Handler

                .word    _sidata
                .word    _sdata
                .word    _edata
                .word    _sbss
                .word    _ebss
/* Reset_Handler */
                .section .text.Reset_Handler
                .weak    Reset_Handler
                .type    Reset_Handler, %function
Reset_Handler:
                ldr      r0, =_estack
                msr      psp, r0

                ldr      r0, =_sdata
                ldr      r1, =_edata
                ldr      r2, =_sidata
                movs     r3, #0
                b        LoopCopyDataInit
CopyDataInit:
                ldr      r4, [r2, r3]
                str      r4, [r0, r3]
                adds     r3, r3, #4
LoopCopyDataInit:
                adds     r4, r0, r3
                cmp      r4, r1
                bcc      CopyDataInit
                
                ldr      r2, =_sbss
                ldr      r4, =_ebss
                movs     r3, #0
                b        LoopFillZerobss
FillZerobss:
                str      r3, [r2]
                adds     r2, r2, #4
LoopFillZerobss:
                cmp      r2, r4
                bcc      FillZerobss

                bl       SystemInit   
                bl       main
                bx       lr    
                .size    Reset_Handler, .-Reset_Handler

/* Default_Handler */
                .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
                b         Infinite_Loop
                .size     Default_Handler, .-Default_Handler
/* Vector Table */
                .section .isr_vector,"a",%progbits
                .type     g_pfnVectors,%object
                .size     g_pfnVectors, .-g_pfnVectors
g_pfnVectors:
                .word     _estack
                .word     Reset_Handler

                .word     NMI_Handler                        /* -14 NMI Handler */
                .word     HardFault_Handler                  /* -13 Hard Fault Handler */
                .word     0                                  /*     Reserved */
                .word     0                                  /*     Reserved */
                .word     0                                  /*     Reserved */
                .word     0                                  /*     Reserved */
                .word     0                                  /*     Reserved */
                .word     0                                  /*     Reserved */
                .word     0                                  /*     Reserved */
                .word     SVC_Handler                        /*  -5 SVCall Handler */
                .word     0                                  /*     Reserved */
                .word     0                                  /*     Reserved */
                .word     PendSV_Handler                     /*  -2 PendSV Handler */
                .word     SysTick_Handler                    /*  -1 SysTick Handler */

                .word     WWDGT_IRQHandler       
                .word     LVD_IRQHandler
                .word     RTC_IRQHandler
                .word     FMC_IRQHandler                    
                .word     RCU_IRQHandler                    
                .word     EXTI0_1_IRQHandler                
                .word     EXTI2_3_IRQHandler                
                .word     EXTI4_15_IRQHandler               
                .word     0                                
                .word     DMA_Channel0_IRQHandler           
                .word     DMA_Channel1_2_IRQHandler         
                .word     DMA_Channel3_4_IRQHandler        
                .word     ADC_CMP_IRQHandler               
                .word     TIMER0_BRK_UP_TRG_COM_IRQHandler 
                .word     TIMER0_Channel_IRQHandler        
                .word     0      
                .word     TIMER2_IRQHandler
                .word     TIMER5_IRQHandler
                .word     0
                .word     TIMER13_IRQHandler
                .word     TIMER14_IRQHandler
                .word     TIMER15_IRQHandler
                .word     TIMER16_IRQHandler
                .word     I2C0_EV_IRQHandler
                .word     I2C1_EV_IRQHandler
                .word     SPI0_IRQHandler
                .word     SPI1_IRQHandler
                .word     USART0_IRQHandler
                .word     USART1_IRQHandler
                .word     0
                .word     0
                .word     0
                .word     I2C0_ER_IRQHandler
                .word     0
                .word     I2C1_ER_IRQHandler

.macro Set_Default_Handler Handler_Name
.weak \Handler_Name
.set  \Handler_Name, Default_Handler
.endm
/*
                Set_Default_Handler  NMI_Handler
                Set_Default_Handler  HardFault_Handler
                Set_Default_Handler  SVC_Handler
                Set_Default_Handler  PendSV_Handler
                Set_Default_Handler  SysTick_Handler
*/
                
                Set_Default_Handler  WWDGT_IRQHandler
                Set_Default_Handler  LVD_IRQHandler
                Set_Default_Handler  RTC_IRQHandler
                Set_Default_Handler  FMC_IRQHandler
                Set_Default_Handler  RCU_IRQHandler
                Set_Default_Handler  EXTI0_1_IRQHandler
                Set_Default_Handler  EXTI2_3_IRQHandler
                Set_Default_Handler  EXTI4_15_IRQHandler
                Set_Default_Handler  DMA_Channel0_IRQHandler
                Set_Default_Handler  DMA_Channel1_2_IRQHandler
                Set_Default_Handler  DMA_Channel3_4_IRQHandler
                Set_Default_Handler  ADC_CMP_IRQHandler
                Set_Default_Handler  TIMER0_BRK_UP_TRG_COM_IRQHandler
                Set_Default_Handler  TIMER0_Channel_IRQHandler
                Set_Default_Handler  TIMER2_IRQHandler
                Set_Default_Handler  TIMER5_IRQHandler
                Set_Default_Handler  TIMER13_IRQHandler
                Set_Default_Handler  TIMER14_IRQHandler
                Set_Default_Handler  TIMER15_IRQHandler
                Set_Default_Handler  TIMER16_IRQHandler
                Set_Default_Handler  I2C0_EV_IRQHandler
                Set_Default_Handler  I2C1_EV_IRQHandler
                Set_Default_Handler  SPI0_IRQHandler
                Set_Default_Handler  SPI1_IRQHandler
                Set_Default_Handler  USART0_IRQHandler
                Set_Default_Handler  USART1_IRQHandler
                Set_Default_Handler  I2C0_ER_IRQHandler
                Set_Default_Handler  I2C1_ER_IRQHandler
.end