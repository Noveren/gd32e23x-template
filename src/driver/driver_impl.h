#ifndef __DRIVER_IMPL_H_
#define __DRIVER_IMPL_H_

#include "gd32e23x.h"

#define __impl_dvr_delay_SYSTICK_CLKSOURCE SYSTICK_CLKSOURCE_HCLK

/// USART0 - GPIOB
#define __impl_dvr_io_USART0_TX_PIN GPIO_PIN_6
#define __impl_dvr_io_USART0_RX_PIN GPIO_PIN_7
#define __impl_dvr_io_USART0_CLOCK RCU_USART0SRC_CKAPB2
#define __impl_dvr_io_USART0_PARITY USART_PM_NONE
#define __impl_dvr_io_USART0_WORD_LENGTH USART_WL_8BIT
#define __impl_dvr_io_USART0_STOP_BIT USART_STB_1BIT
#define __impl_dvr_io_USART0_OVERSAMPLE USART_OVSMOD_8
#define __impl_dvr_io_USART0_BAUDRATE 115200
#define __impl_dvr_io_USART0_TIMEROUT_THRESHOLD 18  /// a magic number based on baudrate 115200
#define __impl_dvr_io_USART0_INT_PRIORITY 3
#define __impl_dvr_io_ringq_CAPACITY 512

/// SPI0 - GPIOA
#define __impl_dvr_spi_SPI0_NSS_PIN GPIO_PIN_4
#define __impl_dvr_spi_SPI0_SCK_PIN GPIO_PIN_5
#define __impl_dvr_spi_SPI0_MISO_PIN GPIO_PIN_6
#define __impl_dvr_spi_SPI0_MOSI_PIN GPIO_PIN_7
#define __impl_dvr_spi_SPI0_FRAMSIZE SPI_FRAMESIZE_8BIT
#define __impl_dvr_spi_SPI0_ENDIAN SPI_ENDIAN_MSB
#define __impl_dvr_spi_SPI0_POLARITY SPI_CK_PL_LOW_PH_1EDGE
#define __impl_dvr_spi_SPI0_CLOCK SPI_PSC_2

/// ADC - GPIOA (PA0, PA1, PA2, PA3)
#define __impl_dvr_adc_ADC_RESOLUTION ADC_RESOLUTION_12B
#define __impl_dvr_adc_ADC_DATAALIGN ADC_DATAALIGN_RIGHT
#define __impl_dvr_adc_ADC_SAMPLETIME ADC_SAMPLETIME_7POINT5

#endif