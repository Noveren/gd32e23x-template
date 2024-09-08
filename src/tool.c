
#include "tool.h"

void tool_init(void) {
    __impl_tool_init();
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

int tool_io_puts(const char* cstr) {
    int counter = 0;
    while (cstr[counter] != '\0') {
        __impl_tool_io_putbyte((const uint8_t)(cstr[counter]));
        counter += 1;
    }
    return counter;
}

void tool_io_putbytes(const uint8_t bytes[], uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        __impl_tool_io_putbyte(bytes[i]);
    }
}

uint16_t tool_io_gets_now(char* buf, uint16_t capacity) {
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
    return len;
}

uint16_t tool_io_gets(char* buf, uint16_t capacity) {
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
    return len;
}