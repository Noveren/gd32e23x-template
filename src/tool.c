
#include "tool.h"

uint32_t tool_strlen(const char* cstr) {
    uint32_t len = 0;
    for (; cstr[len] != '\x0' && len < tool_STRLEN_MAX; len++);
    return len;
}

static uint8_t tool_ascii_number_hex(const uint8_t raw) {
    if (raw > 0x0F) {
        return '?';
    } else {
        return raw <= 0x09 ? raw+0x30 : raw+0x37;
    }
}

#define MAX_UINT32_T 0xFFFFFFFFU
void tool_delay_ms(uint32_t ms) {
    while (ms > 0) {
        if (MAX_UINT32_T / 1000U < ms) {
            __impl_tool_delay_us(MAX_UINT32_T);
            ms -= MAX_UINT32_T / 1000U;
        } else {
            __impl_tool_delay_us(ms * 1000U);
            ms = 0;
        }
    }
}

/// Only transmite the part of cstr, whose length less than tool_STRLEN_MAX
int tool_io_puts(const char* cstr) {
    int len = 0;
    while (cstr[len] != '\0' && len < tool_STRLEN_MAX) {
        __impl_tool_io_putbyte((const uint8_t)(cstr[len]));
        len += 1;
    }
    return len;
}

void tool_io_putbytes(const uint8_t bytes[], uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        __impl_tool_io_putbyte(bytes[i]);
    }
}

const char* tool_io_gets_now(char* buf, uint16_t capacity) {
    uint16_t len = 0;
    if (capacity > 1) {
        int ret = 0;
        do {
            ret = __impl_tool_io_getchar_now();
            if (ret >= 0) {
                buf[len++] = (char)(ret);
            } else {
                break;
            }
        } while (len+1 < capacity);
    }
    if (capacity > 0) {
        buf[len] = '\x0';
    }
    return len == 0 ? NULL : buf;
}

const char* tool_io_gets(char* buf, uint16_t capacity) {
    uint16_t len = 0;
    if (capacity > 1) {
        int ret = 0;
        for (;;) {
            ret = __impl_tool_io_getchar_now();
            if (ret < 0) {
                __impl_tool_delay_us(1000 * 100);
            } else {
                break;
            }
        }
        // FIXME: 若发送速度过快将使得两次数据合并到一起
        // __impl_tool_delay_us(1000 * 100);
        do {
            buf[len++] = (char)(ret);
            ret = __impl_tool_io_getchar_now();
        } while ((ret >= 0) && (len+1 < capacity));
    }
    if (capacity > 0) {
        buf[len] = '\x0';
    }
    return len == 0 ? NULL : buf;
}

void tool_io_putframe_header(uint8_t type, uint32_t len) {
    __impl_tool_io_putbyte(tool_io_FRAME_PREFIX);
    __impl_tool_io_putbyte(type);
    __impl_tool_io_putbyte('0');
    __impl_tool_io_putbyte('x');
    const uint8_t* ptr = (uint8_t*)(&len); // 小端顺序
    for (int i = sizeof(uint32_t)/sizeof(uint8_t); i > 0; i--) {
        __impl_tool_io_putbyte(tool_ascii_number_hex((ptr[i-1] & 0xF0) >> 4));
        __impl_tool_io_putbyte(tool_ascii_number_hex((ptr[i-1] & 0x0F)     ));
    }
}

void tool_io_putframe_footer(void) {
    __impl_tool_io_putbyte(tool_io_FRAME_SUFFIX_0);
    __impl_tool_io_putbyte(tool_io_FRAME_SUFFIX_1);
}

static void tool_io_log(const char* cstr, const char* level) {
    tool_io_putframe_header_text(tool_strlen(level) + tool_strlen(cstr));
    tool_io_puts(level);
    tool_io_puts(cstr);
    tool_io_putframe_footer();
}

#if tool_LOG_LEVEL <= tool_LOG_LEVEL_DEBUG
void tool_io_log_debug(const char *cstr) { tool_io_log(cstr, "DEBUG: "); }
#endif
#if tool_LOG_LEVEL <= tool_LOG_LEVEL_INFO
void tool_io_log_info(const char *cstr) { tool_io_log(cstr, "INFO: "); }
#endif
#if tool_LOG_LEVEL <= tool_LOG_LEVEL_WARN
void tool_io_log_warn(const char *cstr) { tool_io_log(cstr, "WARN: "); }
#endif
#if tool_LOG_LEVEL <= tool_LOG_LEVEL_ERROR
void tool_io_log_error(const char *cstr) { tool_io_log(cstr, "ERROR: "); }
#endif