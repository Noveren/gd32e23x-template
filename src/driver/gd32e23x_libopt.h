#ifndef __GD32E23X_LIBOPT_H
#define __GD32E23X_LIBOPT_H

// #define __SYSTEM_CLOCK_24M_PLL_IRC8M_DIV2        (uint32_t)24000000
#define __SYSTEM_CLOCK_32M_PLL_IRC8M_DIV2        (uint32_t)32000000
// #define __SYSTEM_CLOCK_72M_PLL_IRC8M_DIV2        (uint32_t)72000000

#ifdef GD32E23X_USE_MISC
#include "gd32e23x_misc.h"
#endif
#ifdef GD32E23X_USE_SYSCFG
#include "gd32e23x_syscfg.h"
#endif
#ifdef GD32E23X_USE_RCU
#include "gd32e23x_rcu.h"
#endif
#ifdef GD32E23X_USE_FMC
#include "gd32e23x_fmc.h"
#endif
#ifdef GD32E23X_USE_DMA
#include "gd32e23x_dma.h"
#endif
#ifdef GD32E23X_USE_ADC
#include "gd32e23x_adc.h"
#endif
#ifdef GD32E23X_USE_CRC
#include "gd32e23x_crc.h"
#endif
#ifdef GD32E23X_USE_DBG
#include "gd32e23x_dbg.h"
#endif
#ifdef GD32E23X_USE_EXTI
#include "gd32e23x_exti.h"
#endif
#ifdef GD32E23X_USE_GPIO
#include "gd32e23x_gpio.h"
#endif
#ifdef GD32E23X_USE_I2C
#include "gd32e23x_i2c.h"
#endif
#ifdef GD32E23X_USE_FWDGT
#include "gd32e23x_fwdgt.h"
#endif
#ifdef GD32E23X_USE_PMU
#include "gd32e23x_pmu.h"
#endif
#ifdef GD32E23X_USE_RTC
#include "gd32e23x_rtc.h"
#endif
#ifdef GD32E23X_USE_SPI
#include "gd32e23x_spi.h"
#endif
#ifdef GD32E23X_USE_TIMER
#include "gd32e23x_timer.h"
#endif
#ifdef GD32E23X_USE_USART
#include "gd32e23x_usart.h"
#endif
#ifdef GD32E23X_USE_WWDGT
#include "gd32e23x_wwdgt.h"
#endif
#ifdef GD32E23X_USE_CMP
#include "gd32e23x_cmp.h"
#endif

#endif
