#ifndef __APP_H_
#define __APP_H_

#include <stdbool.h>

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
const char* app_gets_or_NULL(void);
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

typedef struct {
    uint8_t manufacture_id[7];
    uint16_t framily: 3;
    uint16_t density: 5;
    uint16_t sub    : 2;
    uint16_t rev    : 3;
    uint16_t rsvd   : 3;
} app_FRAM_ID;

typedef enum {
    app_FRAM_OPCODE_NOP   = 0b00000000,
    app_FRAM_OPCODE_WREN  = 0b00000110,
    app_FRAM_OPCODE_WRDI  = 0b00000100,
    app_FRAM_OPCODE_RDSR  = 0b00000101,
    app_FRAM_OPCODE_WRSR  = 0b00000001,
    app_FRAM_OPCODE_READ  = 0b00000011,
    app_FRAM_OPCODE_FSTRD = 0b00001011,
    app_FRAM_OPCODE_WRITE = 0b00000010,
    app_FRAM_OPCODE_SLEEP = 0b10111001,
    app_FRAM_OPCODE_RDID  = 0b10011111,
} app_FRAM_OPCODE;

/// TODO 支持自动识别铁电存储器型号
#define app_FRAM_CAPACITY 65536

typedef bool(*CommandFn)(void);

bool app_cmd_get_fram_id(void);
bool app_cmd_get_fram_status_register(void);
bool app_cmd_get_fram_data(void);
bool app_cmd_set_fram_write_enable(void);
bool app_cmd_set_fram_clean(void);
bool app_cmd_get_adc_once(void);
bool app_cmd_set_adc_timer_start(void);
bool app_cmd_collect_signal(void);
bool app_cmd_collect_signal_with_triger(void);
bool app_cmd_test_timer(void);

#endif