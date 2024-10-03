
#include "driver.h"

#define MAX_UINT32_T 0xFFFFFFFFU
void dvr_delay_ms(uint32_t ms) {
    while (ms > 0) {
        if (MAX_UINT32_T / 1000U < ms) {
            __impl_dvr_delay_us(MAX_UINT32_T);
            ms -= MAX_UINT32_T / 1000U;
        } else {
            __impl_dvr_delay_us(ms * 1000U);
            ms = 0;
        }
    }
}
#undef MAX_UINT32_T

void dvr_io_putbytes(const uint8_t* bytes, uint16_t len) {
    for (uint32_t i = 0; i < len; i++) {
        __impl_dvr_io_put(bytes[i]);
    }
}

void dvr_io_putbytes_text(const uint8_t* bytes, uint16_t len, char seq) {
    uint16_t ascii = 0x0000;
    for (uint16_t i = 0; i < len; i++) {
        ascii = util_byte2ascii(bytes[i]);
        __impl_dvr_io_put((uint8_t)((ascii & 0xFF00) >> 8));
        __impl_dvr_io_put((uint8_t)((ascii & 0x00FF)     ));
        if (seq && i < len-1) {
            __impl_dvr_io_put((uint8_t)seq);
        }
    }
}

void dvr_io_putbytes_text_reverse(const uint8_t* bytes, uint16_t len, char seq) {
    uint16_t ascii = 0x0000;
    for (uint16_t i = len; i > 0; i--) {
        ascii = util_byte2ascii(bytes[i-1]);
        __impl_dvr_io_put((uint8_t)((ascii & 0xFF00) >> 8));
        __impl_dvr_io_put((uint8_t)((ascii & 0x00FF)     ));
        if (seq && i > 1) {
            __impl_dvr_io_put((uint8_t)seq);
        }
    }
}

uint16_t dvr_io_puts(const char* cstr) {
    uint16_t len = 0;
    while (cstr[len] != '\0' && len < util_CSTR_MAX_LENGTH) {
        __impl_dvr_io_put((uint8_t)(cstr[len]));
        len += 1;
    }
    return len;
}

const char* dvr_io_gets_or_NULL(char* buf, uint16_t len) {
    uint16_t counter = 0;
    if (len > 1) {
        int ret = 0;
        do {
            if ((ret = __impl_dvr_io_get()) >= 0) {
                buf[counter++] = (char)(0x000000FF & ret);
            } else {
                break;
            }
        } while (counter+1 < len);
    }
    if (len > 0) {
        buf[counter] = '\x00';
    }
    return counter == 0 ? NULL : buf;
}

const char* dvr_io_gets(char* buf, uint16_t len) {
    uint16_t counter = 0;
    if (len > 1) {
        int ret = 0;
        for (;;) {
            if ((ret = __impl_dvr_io_get()) < 0) {
                __impl_dvr_delay_us(1000 * 10);
            } else {
                break;
            }
        }
        do {
            buf[counter++] = (char)(0x000000FF & ret);
        } while (((ret = __impl_dvr_io_get()) >= 0) && (counter+1 < len));
    }
    if (len > 0) {
        buf[counter] = '\x00';
    }
    return buf;
}

const char* dvr_io_gets_within_x_sec_or_NULL(char* buf, uint16_t len, uint8_t sec) {
    const char* input = NULL;
    for (uint16_t n100ms = sec * 10; n100ms > 0; n100ms--) {
        input = dvr_io_gets_or_NULL(buf, len);
        if (input != NULL) {
            break;
        }
        dvr_delay_ms(100);
    }
    return input;
}