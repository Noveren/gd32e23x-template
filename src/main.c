
// TODO 在 FLASH 中保存程序装定信息（可读写）

#include "main.h"

static const char* app_get_input_within_x_mins(char* buf, const uint16_t capacity, const uint32_t min);
static int app_strcmp(const char* str1, const char* str2);

/// 可以并能够借用 `app_init` 中初始化后的资源；其他资源使用后应该恢复原状
typedef void(*app_TaskFn)(void);
static void app_task_get_fram_id(void);
static void app_task_get_fram_status_register(void);
static void app_task_get_fram_data(void);
static void app_task_set_fram_write_enable(void);
static void app_task_set_fram_clean(void);
static void app_task_get_adc_once(void);
static void app_task_set_adc_timer_start(void);
static void app_task_collect_signal(void);

static app_TaskFn app_TASK_TABLE[] = {
    ///
    app_task_get_fram_id,
    ///
    app_task_get_fram_status_register,
    ///
    app_task_get_fram_data,
    ///
    app_task_set_fram_write_enable,
    ///
    app_task_set_fram_clean,
    ///
    app_task_get_adc_once,
    ///
    app_task_set_adc_timer_start,
    ///
    app_task_collect_signal,
};

static int8_t app_parse_command(const char* input) {
    if (0 == app_strcmp(input, "<!get_fram_id>"))
        return 0;
    if (0 == app_strcmp(input, "<!get_fram_status_register>"))
        return 1;
    if (0 == app_strcmp(input, "<!get_fram_data>"))
        return 2;
    if (0 == app_strcmp(input, "<!set_fram_write_enable>"))
        return 3;
    if (0 == app_strcmp(input, "<!set_fram_clean>"))
        return 4;
    if (0 == app_strcmp(input, "<!get_adc_once>"))
        return 5;
    if (0 == app_strcmp(input, "<!set_adc_timer_start>"))
        return 6;
    if (0 == app_strcmp(input, "<!collect_signal>"))
        return 7;
    return -1;
}

#define APP_INPUT_BUF_SIZE 128
void main() {
    tool_init();
    tool_io_enable();

    tool_delay_ms(2000);
    tool_led_on();

    char input_buf[APP_INPUT_BUF_SIZE] = { 0 };
    const char* input = NULL;

    tool_io_log_debug("Wait for input within 1 minute, otherwise the default task will be executed.");
    input = app_get_input_within_x_mins(input_buf, APP_INPUT_BUF_SIZE, 1);

    if (input == NULL) {
        tool_io_log_debug("No input.");
        tool_io_log_warn("Execute the default task.");

        tool_io_log_debug("Wait for RTC alarm");
        // TODO 时间不够精准 0:30->0:36, 1:30->1:36, 0:10->0:11
        tool_deepsleep_with_rtc(0x0, 0x01, 0x0);
        tool_io_log_debug("RTC alarm");
        tool_led_off();
        while (1) {
            tool_delay_ms(1000);
            tool_led_toggle();
        }
    } else {
        int8_t cmd_idx = -1;
        do {
            if ((cmd_idx = app_parse_command(input)) < 0) {
                tool_io_log_info("Invaild Command.");
                tool_io_log_info(input);
            } else {
                app_TASK_TABLE[cmd_idx]();
            }
            input = tool_io_gets(input_buf, APP_INPUT_BUF_SIZE);
        } while (1);
    }
    tool_io_log_warn("System trun off.");
}
#undef APP_INPUT_BUF_SIZE

/// Wait for input (no more than `capacity-1`) within x mins,
/// otherwise the return value will be `NULL`.
static const char* app_get_input_within_x_mins(char* buf, const uint16_t capacity, const uint32_t min) {
    uint16_t len = 0;
    const char* input = NULL;
    for (uint32_t n100ms = min * 600; n100ms > 0; n100ms--) {
        input = tool_io_gets_now(buf, 128);
        if (input == NULL) {
            tool_delay_ms(100);
        } else {
            break;
        }
    }
    return input;
}

static int app_strcmp(const char* str1, const char* str2) {
    int ret = 0;
    while ((ret = *(unsigned char*)str1 - *(unsigned char*)str2) == 0 && *str1 && *str2) {
        str1 += 1;
        str2 += 1;
    }
    return ret;
}

static void app_task_get_fram_id(void) {
    tool_io_log_info("Execute `app_task_get_fram_id`.");

    app_FRAM_ID id = { 0 };

    tool_spi_init();
    tool_spi_enable();
    tool_spi_select();

    tool_spi_access_data(app_FRAM_OPCODE_RDID);
    for (int i = 0; i < 9; i++) {
        ((uint8_t*)(&id))[i] = tool_spi_access_data(app_FRAM_OPCODE_NOP);
    }

    tool_spi_release();
    tool_spi_disable();
    tool_spi_deinit();

    tool_io_putframe_text_bytes((uint8_t*)&id, 9);
}

static void app_task_get_fram_status_register(void) {
    tool_io_log_info("Execute `app_task_get_fram_status_register`.");

    uint8_t reg = 0;

    tool_spi_init();
    tool_spi_enable();
    tool_spi_select();

    tool_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = tool_spi_access_data(app_FRAM_OPCODE_NOP);

    tool_spi_release();
    tool_spi_disable();
    tool_spi_deinit();

    tool_io_putframe_text_bytes(&reg, 1);
}

static void app_task_get_fram_data(void) {
    tool_io_log_info("Execute `app_task_get_fram_data`.");

    tool_spi_init();
    tool_spi_enable();
    tool_spi_select();
    tool_io_putframe_header_data(app_FRAM_CAPACITY);

    tool_spi_access_data(app_FRAM_OPCODE_READ);
    tool_spi_access_data(0x00);
    tool_spi_access_data(0x00);
    uint8_t byte = 0;
    // for (uint32_t i = 0; i < app_FRAM_CAPACITY; i++) {
    for (uint32_t i = 0; i < 100; i++) {
        byte = tool_spi_access_data(app_FRAM_OPCODE_NOP);
        tool_io_putbyte(byte);
    }

    tool_io_putframe_footer();
    tool_spi_release();
    tool_spi_disable();
    tool_spi_deinit();
}

static void app_task_set_fram_write_enable(void) {
    tool_io_log_info("Execute `app_task_set_fram_write_enable`.");

    tool_spi_init();

    tool_spi_enable();
    tool_spi_select();
    tool_spi_access_data(app_FRAM_OPCODE_WREN);
    tool_spi_release();
    tool_spi_disable();

    tool_delay_ms(1);

    uint8_t reg = 0;
    tool_spi_enable();
    tool_spi_select();
    tool_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = tool_spi_access_data(app_FRAM_OPCODE_NOP);
    tool_spi_release();
    tool_spi_disable();

    tool_spi_deinit();

    tool_io_putframe_text_bytes(&reg, 1);
}

static void app_task_set_fram_clean(void) {
    tool_io_log_info("Execute `app_task_set_fram_clean`.");

    tool_spi_init();

    uint8_t reg = 0;
    tool_spi_enable();
    tool_spi_select();
    tool_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = tool_spi_access_data(app_FRAM_OPCODE_NOP);
    tool_spi_release();
    tool_spi_disable();

    if ((reg & 0x02) == 0) {
        tool_io_putframe_header_text(1);
        tool_io_putbyte('1');
        tool_io_putframe_footer();
    } else {
        tool_spi_enable();
        tool_spi_select();
        tool_spi_access_data(app_FRAM_OPCODE_WRITE);
        tool_spi_access_data(0x00);
        tool_spi_access_data(0x00);
        for (uint32_t i = 0; i < app_FRAM_CAPACITY; i++) {
            tool_spi_access_data(0xFF);
        }
        tool_spi_release();
        tool_spi_disable();

        tool_io_putframe_header_text(1);
        tool_io_putbyte('0');
        tool_io_putframe_footer();
    }

    tool_spi_deinit();
}

static void app_task_get_adc_once(void) {
    tool_io_log_info("Execute `app_task_get_adc_once`.");

    tool_adc_init(tool_adc_CHANNEL_0 | tool_adc_CHANNEL_1 | tool_adc_CHANNEL_2 | tool_adc_CHANNEL_3);
    // tool_adc_init(tool_adc_CHANNEL_0 | tool_adc_CHANNEL_2);

    if (tool_adc_convert_once_async()) {
        const uint16_t* result = NULL;
        for (;;) {
            if ((result = tool_adc_get_result()) != NULL) {
                tool_io_putframe_text_bytes((const uint8_t *)result, 8);
                break;
            }
        }
    } else {
        tool_io_log_error("Failed to start a conversion.");
    }
    tool_adc_deinit();
}

static bool tool_timer_callbackfn(void* _) {
    return tool_adc_convert_once_async();
}

static void app_task_set_adc_timer_start(void) {
    tool_io_log_info("Execute `app_task_set_adc_timer_start`.");

    tool_adc_init(tool_adc_CHANNEL_0 | tool_adc_CHANNEL_1 | tool_adc_CHANNEL_2 | tool_adc_CHANNEL_3);

    tool_timer_init(tool_timer_freq_100us);
    tool_timer_enable(10000, tool_timer_callbackfn);
    const uint16_t* result = NULL;
    for (;;) {
        if (!tool_io_getchar_is_empty()) {
            break;
        }
        if ((result = tool_adc_get_result()) != NULL) {
            tool_io_putframe_text_bytes((const uint8_t *)result, 8);
        }
    }
    tool_timer_disable();
    tool_timer_deinit();

    tool_adc_deinit();
}

static void app_task_collect_signal(void) {
    tool_io_log_info("Execute `app_task_collect_signal`.");

    tool_spi_init();

    uint8_t reg = 0;
    tool_spi_enable();
    tool_spi_select();
    tool_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = tool_spi_access_data(app_FRAM_OPCODE_NOP);
    tool_spi_release();
    tool_spi_disable();
    if ((reg & 0x02) == 0) {
        tool_io_putframe_header_text(1);
        tool_io_putbyte('1');
        tool_io_putframe_footer();
    } else {
        tool_spi_enable();
        tool_spi_select();
        tool_spi_access_data(app_FRAM_OPCODE_WRITE);
        tool_spi_access_data(0x00);
        tool_spi_access_data(0x00);


        tool_adc_init(tool_adc_CHANNEL_0 | tool_adc_CHANNEL_1 | tool_adc_CHANNEL_2 | tool_adc_CHANNEL_3);

        const uint16_t* result = NULL;
        uint16_t result_buf[4] = { 0 };
        uint16_t counter = 0;
        tool_timer_init(tool_timer_freq_10us);
        tool_timer_enable(1, tool_timer_callbackfn);
        for (;;) {
            if (counter >= 8192 || !tool_timer_is_working()) {
                break;
            }
            if ((result = tool_adc_get_result()) != NULL) {
                counter += 1;
                /// 缓存结果
                result_buf[0] = result[0];
                result_buf[1] = result[1];
                result_buf[2] = result[2];
                result_buf[3] = result[3];
                /// 存入 FRAM
                for (uint8_t i = 0; i < 8; i++) {
                    tool_spi_access_data(((const uint8_t*)result_buf)[i]);
                }
            }
        }
        tool_timer_disable();
        tool_timer_deinit();

        tool_spi_release();
        tool_spi_disable();

        tool_adc_deinit();

        tool_io_putframe_header_text(1);
        tool_io_putbyte('0');
        tool_io_putframe_footer();
    }
    tool_spi_deinit();
}