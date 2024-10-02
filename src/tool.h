/**
 * 阻塞延时、数据收发
 * 平台需要实现以下函数
 */
/// 阻塞延时、数据收发
/// 平台需要实现以下函数

#ifndef __TOOL_H_
#define __TOOL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef bool (*CallbackFn)(void *);

/// 当字符串长度超过最大值时，相关函数行为异常
#define tool_STRLEN_MAX 256

uint32_t tool_strlen(const char* cstr);

uint8_t tool_utils_byte2number_0x0F(uint8_t byte);
uint16_t tool_utils_byte2ascii(uint8_t byte);

#define NEED_IMPL extern

NEED_IMPL void __impl_tool_delay_init(void);
NEED_IMPL void __impl_tool_io_init(void);

NEED_IMPL void __impl_tool_spi_init(void);
NEED_IMPL void __impl_tool_spi_deinit(void);

/// CHANNEL_0 ~ CHANNEL_3 对应 PA0 ~ PA3
#define tool_adc_CHANNEL_0 0x01
#define tool_adc_CHANNEL_1 0x02
#define tool_adc_CHANNEL_2 0x04
#define tool_adc_CHANNEL_3 0x08

NEED_IMPL void __impl_tool_adc_init(const uint8_t tool_adc_CHANNEL);
NEED_IMPL void __impl_tool_adc_deinit(void);

/// ck / (ck / step)
#define tool_timer_freq_100us 10000
#define tool_timer_freq_10us  100000
NEED_IMPL void __impl_tool_timer_init(uint32_t freq);
NEED_IMPL void __impl_tool_timer_deinit(void);

/// 初始化 `tool_delay` 和 `tool_io` 并保持全局可用
/// 其他功能可直接使用或需要单独初始化：
///
/// + `tool_deepsleep`: 直接使用
inline void tool_init(void) {
    __impl_tool_delay_init();
    __impl_tool_io_init();
}

/// ===============================================================

/// 微秒阻塞延时实现
NEED_IMPL void __impl_tool_delay_us(uint32_t us);
/// TODO
inline void tool_delay_us(uint32_t us) { __impl_tool_delay_us(us); }
/// TODO
void tool_delay_ms(uint32_t ms);

/// ===============================================================

NEED_IMPL void __impl_tool_io_enable(void);
NEED_IMPL void __impl_tool_io_disable(void);
#define tool_io_enable() do { __impl_tool_io_enable(); } while (0)
#define tool_io_disable() do { __impl_tool_io_disable(); } while (0)

/// 阻塞发送一个 byte 数据
NEED_IMPL void __impl_tool_io_putbyte(const uint8_t byte);
inline void tool_io_putbyte(const uint8_t byte) { __impl_tool_io_putbyte(byte); }
void tool_io_putword(const uint16_t word);
inline void tool_io_putchar(const char ch) { __impl_tool_io_putbyte((uint8_t)(ch)); }
int tool_io_puts(const char* cstr);
void tool_io_putbytes(const uint8_t bytes[], uint32_t len);
void tool_io_putbytes_reverse(const uint8_t bytes[], uint32_t len);
void tool_io_putbytes_text(const uint8_t bytes[], uint32_t len, char prefix);
void tool_io_putbytes_reverse_text(const uint8_t bytes[], uint32_t len, char prefix);

NEED_IMPL bool __impl_tool_io_getchar_is_empty(void);
inline bool tool_io_getchar_is_empty(void) { return __impl_tool_io_getchar_is_empty(); }

/// 立刻获得一个字节数据，若无数据，则返回值小于 0
NEED_IMPL int __impl_tool_io_getchar_now(void);

const char* tool_io_gets_now(char* buf, uint16_t capacity);
const char* tool_io_gets(char* buf, uint16_t capacity);

void tool_io_putframe_header(uint8_t type, uint32_t len);
void tool_io_putframe_footer(void);

#define tool_io_FRAME_TYPE_CMD  '!'
#define tool_io_FRAME_TYPE_TEXT '"'
#define tool_io_FRAME_TYPE_DATA '$'
#define tool_io_FRAME_PREFIX    '<'
#define tool_io_FRAME_SUFFIX_0  '>'
#define tool_io_FRAME_SUFFIX_1  '\n'
inline void tool_io_putframe_header_text(uint32_t len) { tool_io_putframe_header(tool_io_FRAME_TYPE_TEXT, len); }
inline void tool_io_putframe_header_data(uint32_t len) { tool_io_putframe_header(tool_io_FRAME_TYPE_DATA, len); }
void tool_io_putframe_text_bytes(const uint8_t bytes[], uint32_t len);

#define tool_LOG_LEVEL_DEBUG 0
#define tool_LOG_LEVEL_INFO  1
#define tool_LOG_LEVEL_WARN  2
#define tool_LOG_LEVEL_ERROR 3

#ifndef tool_LOG_LEVEL
    #ifdef DEBUG
        #define tool_LOG_LEVEL tool_LOG_LEVEL_DEBUG
    #else
        #define tool_LOG_LEVEL tool_LOG_LEVEL_INFO
    #endif
#endif

#if tool_LOG_LEVEL <= tool_LOG_LEVEL_DEBUG
    void tool_io_log_debug(const char* cstr);
#else
    #define tool_io_log_debug(cstr) do { /* None */ } while (0)
#endif

#if tool_LOG_LEVEL <= tool_LOG_LEVEL_INFO
    void tool_io_log_info(const char* cstr);
#else
    #define tool_io_log_info(cstr) do { /* None */ } while (0)
#endif

#if tool_LOG_LEVEL <= tool_LOG_LEVEL_WARN
    void tool_io_log_warn(const char* cstr);
#else
    #define tool_io_log_debug(cstr) do { /* None */ } while (0)
#endif

#if tool_LOG_LEVEL <= tool_LOG_LEVEL_ERROR
    void tool_io_log_error(const char* cstr);
#else
    #define tool_io_log_error(cstr) do { /* None */ } while (0)
#endif

/// ===============================================================

NEED_IMPL void __impl_tool_deepsleep_with_rtc(uint8_t hour, uint8_t minute, uint8_t second);
/// 参数均采用 BCD 格式, 采用 24 小时制, 赋值范围位 0x0-0x23, 0x0-0x59, 0x0-0x59
inline void tool_deepsleep_with_rtc(uint8_t hour, uint8_t minute, uint8_t second) { __impl_tool_deepsleep_with_rtc(hour, minute, second); }

/// ===============================================================

#define tool_spi_init() do { __impl_tool_spi_init(); } while (0)
#define tool_spi_deinit() do { __impl_tool_spi_deinit(); } while (0)

NEED_IMPL void __impl_tool_spi_enable(void);
NEED_IMPL void __impl_tool_spi_disable(void);
#define tool_spi_enable() do { __impl_tool_spi_enable(); } while (0)
#define tool_spi_disable() do { __impl_tool_spi_disable(); } while (0)

NEED_IMPL void __impl_tool_spi_select(void);
NEED_IMPL void __impl_tool_spi_release(void);
#define tool_spi_select() do { __impl_tool_spi_select(); } while (0)
#define tool_spi_release() do { __impl_tool_spi_release(); } while (0)

NEED_IMPL uint8_t __impl_tool_spi_access_data(uint8_t byte);
inline uint8_t tool_spi_access_data(uint8_t byte) { return __impl_tool_spi_access_data(byte); }

/// ===============================================================

inline void tool_adc_init(const uint8_t tool_adc_CHANNEL) { __impl_tool_adc_init(tool_adc_CHANNEL); }
#define tool_adc_deinit() do { __impl_tool_adc_deinit(); } while (0)

NEED_IMPL bool __impl_tool_adc_convert_once_async(void);
inline bool tool_adc_convert_once_async(void) { return __impl_tool_adc_convert_once_async(); }

NEED_IMPL const uint16_t* __impl_tool_adc_get_result(void);
inline const uint16_t* tool_adc_get_result(void) { return __impl_tool_adc_get_result(); }

/// ===============================================================

inline void tool_timer_init(uint32_t freq) { __impl_tool_timer_init(freq); }
#define tool_timer_deinit() do { __impl_tool_timer_deinit(); } while (0)

NEED_IMPL bool __impl_tool_timer_enable(uint16_t autoreload, CallbackFn fn);
inline bool tool_timer_enable(uint16_t autoreload, CallbackFn fn) { return __impl_tool_timer_enable(autoreload, fn); }
NEED_IMPL void __impl_tool_timer_disable(void);
#define tool_timer_disable() do { __impl_tool_timer_disable(); } while (0)
NEED_IMPL bool __impl_tool_timer_is_working(void);
inline bool tool_timer_is_working(void) { return __impl_tool_timer_is_working(); }

#undef NEED_IMPL
#endif