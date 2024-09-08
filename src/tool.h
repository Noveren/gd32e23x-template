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

#define NEED_IMPL extern

NEED_IMPL void __impl_tool_init(void);
void tool_init(void);

/// 微秒阻塞延时实现
NEED_IMPL void __impl_tool_delay_us(uint32_t us);
inline void tool_delay_us(uint32_t us) { __impl_tool_delay_us(us); }
void tool_delay_ms(uint32_t ms);

NEED_IMPL void __impl_tool_io_enable(void);
NEED_IMPL void __impl_tool_io_disable(void);
#define tool_io_enable() do { __impl_tool_io_enable(); } while (0)
#define tool_io_disable() do { __impl_tool_io_disable(); } while (0)

/// 阻塞发送一个 byte 数据
NEED_IMPL void __impl_tool_io_putbyte(const uint8_t byte);
inline void tool_io_putbyte(const uint8_t byte) { __impl_tool_io_putbyte(byte); }
inline void tool_io_putchar(const char ch) { __impl_tool_io_putbyte((uint8_t)(ch)); }
int tool_io_puts(const char* cstr);
void tool_io_putbytes(const uint8_t bytes[], uint32_t len);

/// 立刻获得一个字节数据，若无数据，则返回值小于 0
NEED_IMPL int __impl_tool_io_getchar_now(void);

uint16_t tool_io_gets_now(char* buf, uint16_t capacity);
uint16_t tool_io_gets(char* buf, uint16_t capacity);
// void tool_io_putframe_cstr(const char* cstr);
// void tool_io_putframeheader_data(void);
// void tool_io_putframe_data()



#undef NEED_IMPL
#endif