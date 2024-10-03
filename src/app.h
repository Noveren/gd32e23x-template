#ifndef __APP_H_
#define __APP_H_

#include "util.h"
#include "gd32e23x.h"

#define APP_INPUT_BUF_SIZE 128

void app_init(void);
void app_deinit(void);

__STATIC_FORCEINLINE void app_led_on(void)     { GPIO_BC(GPIOA)  = (uint32_t)(GPIO_PIN_15); }
__STATIC_FORCEINLINE void app_led_off(void)    { GPIO_BOP(GPIOA) = (uint32_t)(GPIO_PIN_15); }
__STATIC_FORCEINLINE void app_led_toggle(void) { GPIO_TG(GPIOA)  = (uint32_t)(GPIO_PIN_15); }

void app_delay_ms(uint32_t ms);
const char* app_gets(void);
const char* app_gets_within_x_sec_or_NULL(uint8_t sec);

void app_deepsleep_with_rtc(uint8_t hour, uint8_t minute, uint8_t second);

#define app_FRAME_PREFIX    '<'
#define app_FRAME_TYPE_CMD  '!'
#define app_FRAME_TYPE_TEXT '#'
#define app_FRAME_TYPE_DATA '$'
void app_putframe_header(char type, uint32_t len);
__STATIC_FORCEINLINE void app_putframe_header_text(uint32_t len) { app_putframe_header(app_FRAME_TYPE_TEXT, len); }
__STATIC_FORCEINLINE void app_putframe_header_data(uint32_t len) { app_putframe_header(app_FRAME_TYPE_DATA, len); }
#define app_FRAME_SUFFIX_0  '>'
#define app_FRAME_SUFFIX_1  '\n'
void app_putframe_footer(void);

void app_print(const char* cstr);

#ifdef DEBUG
    void app_log_debug(const char* cstr);
#else
    #define app_log_debug(cstr) do { /* None */ } while (0)
#endif

#endif