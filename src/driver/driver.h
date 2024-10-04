#ifndef __DRIVER_H_
#define __DRIVER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "util.h"

#define NEED_IMPL extern
#define WEEK_IMPL extern

NEED_IMPL void __impl_dvr_delay_init(void);
NEED_IMPL void __impl_dvr_delay_deinit(void);
NEED_IMPL void __impl_dvr_delay_us(uint32_t us);

NEED_IMPL void __impl_dvr_io_init(void);
NEED_IMPL void __impl_dvr_io_deinit(void);
NEED_IMPL void __impl_dvr_io_put(uint8_t byte);
NEED_IMPL void __impl_dvr_io_get_enable(void);
NEED_IMPL void __impl_dvr_io_get_disable(void);
// NEED_IMPL bool __impl_dvr_io_get_check(void);
NEED_IMPL int  __impl_dvr_io_get(void);

NEED_IMPL void __impl_dvr_deepsleep_with_rtc(uint8_t hour, uint8_t minute, uint8_t second);

NEED_IMPL void __impl_dvr_spi_init(void);
NEED_IMPL void __impl_dvr_spi_deinit(void);
NEED_IMPL void __impl_dvr_spi_enable(void);
NEED_IMPL void __impl_dvr_spi_disable(void);
NEED_IMPL void __impl_dvr_spi_select(void);
NEED_IMPL void __impl_dvr_spi_release(void);
NEED_IMPL uint8_t __impl_dvr_spi_access_data(uint8_t byte);

typedef enum {
    dvr_adc_CHANNEL_0 = 0x01,
    dvr_adc_CHANNEL_1 = 0x02,
    dvr_adc_CHANNEL_2 = 0x04,
    dvr_adc_CHANNEL_3 = 0x08,
} dvr_adc_CHANNEL;
NEED_IMPL void __impl_dvr_adc_init(const uint8_t dvr_adc_CHANNEL);
NEED_IMPL void __impl_dvr_adc_deinit(void);
NEED_IMPL bool __impl_dvr_adc_convert_once_async(void);
NEED_IMPL const volatile uint16_t* __impl_dvr_adc_get_result(void);

#define dvr_timer_FREQ_100US 10000
#define dvr_timer_FREQ_50US  20000
#define dvr_timer_FREQ_20US  50000
#define dvr_timer_FREQ_10US  100000
#define dvr_timer_FREQ_5US   200000
NEED_IMPL bool __impl_dvr_timer_init(uint32_t dvr_timer_FREQ);
NEED_IMPL void __impl_dvr_timer_deinit(void);
NEED_IMPL bool __impl_dvr_timer_enable(uint16_t step);
NEED_IMPL void __impl_dvr_timer_disable(void);
NEED_IMPL bool __impl_dvr_timer_is_working(void);
WEEK_IMPL bool dvr_timer_callback(void* _);

#undef WEEK_IMPL
#undef NEED_IMPL

/// ===============================================================

__STATIC_FORCEINLINE void dvr_init(void)   { __impl_dvr_delay_init(); __impl_dvr_io_init();     }
__STATIC_FORCEINLINE void dvr_deinit(void) { __impl_dvr_delay_deinit(); __impl_dvr_io_deinit(); }

__STATIC_FORCEINLINE void dvr_delay_us(uint32_t us) { __impl_dvr_delay_us(us); }
void dvr_delay_ms(uint32_t ms);

__STATIC_FORCEINLINE void dvr_io_putbyte(uint8_t byte) { __impl_dvr_io_put(byte); }
__STATIC_FORCEINLINE void dvr_io_putchar(char ch) { __impl_dvr_io_put((uint8_t)ch); }
void dvr_io_putbytes(const uint8_t* bytes, uint16_t len);
void dvr_io_putbytes_text(const uint8_t* bytes, uint16_t len, char seq);
void dvr_io_putbytes_text_reverse(const uint8_t* bytes, uint16_t len, char seq);
uint16_t dvr_io_puts(const char* cstr);
__STATIC_FORCEINLINE void dvr_io_get_enable(void) { __impl_dvr_io_get_enable(); }
__STATIC_FORCEINLINE void dvr_io_get_disable(void) { __impl_dvr_io_get_disable(); }
const char* dvr_io_gets_or_NULL(char* buf, uint16_t len);
const char* dvr_io_gets(char* buf, uint16_t len);
const char* dvr_io_gets_within_x_sec_or_NULL(char* buf, uint16_t len, uint8_t sec);

/// ===============================================================

__STATIC_FORCEINLINE void dvr_deepsleep_with_rtc(uint8_t hour, uint8_t minute, uint8_t second) { __impl_dvr_deepsleep_with_rtc(hour, minute, second); }

/// ===============================================================

__STATIC_FORCEINLINE void dvr_spi_init(void) { __impl_dvr_spi_init(); }
__STATIC_FORCEINLINE void dvr_spi_deinit(void) { __impl_dvr_spi_deinit(); }
__STATIC_FORCEINLINE void dvr_spi_enable(void) { __impl_dvr_spi_enable(); }
__STATIC_FORCEINLINE void dvr_spi_disable(void) { __impl_dvr_spi_disable(); }
__STATIC_FORCEINLINE void dvr_spi_select(void) { __impl_dvr_spi_select(); }
__STATIC_FORCEINLINE void dvr_spi_release(void) { __impl_dvr_spi_release(); }
__STATIC_FORCEINLINE uint8_t dvr_spi_access_data(uint8_t byte) { return __impl_dvr_spi_access_data(byte); }

/// ===============================================================

__STATIC_FORCEINLINE void dvr_adc_init(const uint8_t dvr_adc_CHANNEL) { __impl_dvr_adc_init(dvr_adc_CHANNEL); }
__STATIC_FORCEINLINE void dvr_adc_deinit(void) { __impl_dvr_adc_deinit(); }
__STATIC_FORCEINLINE bool dvr_adc_convert_once_async(void) { return __impl_dvr_adc_convert_once_async(); }
__STATIC_FORCEINLINE const volatile uint16_t* dvr_adc_get_result(void) { return __impl_dvr_adc_get_result(); }

/// ===============================================================

__STATIC_FORCEINLINE bool dvr_timer_init(uint32_t dvr_timer_FREQ) { return __impl_dvr_timer_init(dvr_timer_FREQ); }
__STATIC_FORCEINLINE void dvr_timer_deinit(void) { __impl_dvr_timer_deinit(); }
__STATIC_FORCEINLINE bool dvr_timer_enable(uint16_t step) { return __impl_dvr_timer_enable(step); }
__STATIC_FORCEINLINE void dvr_timer_disable(void) { __impl_dvr_timer_disable(); }
__STATIC_FORCEINLINE bool dvr_timer_is_working(void) { return __impl_dvr_timer_is_working(); }

/// ===============================================================

#endif