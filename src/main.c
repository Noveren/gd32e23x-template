
// TODO 协议帧头
// TODO 在 FLASH 中保存程序装定信息（可读写）
// TODO FRAM 访问
// TODO ADC 采集
// TODO 基于串口的任务选择执行框架

#include "main.h"

#define RECEIVE_BUF_MAX_LEN 128

void app_init(void);

void app_init(void) {
    /* pa15 init */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);

    tool_init();
    tool_io_enable();
}

#define app_led_on()     do { GPIO_BC(GPIOA)  = (uint32_t)(GPIO_PIN_15); } while (0)
#define app_led_off()    do { GPIO_BOP(GPIOA) = (uint32_t)(GPIO_PIN_15); } while (0)
#define app_led_toggle() do { GPIO_TG(GPIOA)  = (uint32_t)(GPIO_PIN_15); } while (0)

const char* app_get_input_within_x_mins(char* buf, const uint16_t capacity, const uint32_t min);
static int app_strcmp(const char* str1, const char* str2);

/// 可以并能够借用 `app_init` 中初始化后的资源；其他资源使用后应该恢复原状
typedef void(*app_TaskFn)(void);
void app_task_get_info(void);
void app_task_get_data(void);

static app_TaskFn app_TASK_TABLE[] = {
    app_task_get_info,
    app_task_get_data,
};

int8_t app_parse_command(const char* input) {
    if (0 == app_strcmp(input, "<!info>"))
        return 0;
    if (0 == app_strcmp(input, "<!data>"))
        return 1;
    return -1;
}

#define APP_INPUT_BUF_SIZE 128

void main() {
    app_init();

    tool_delay_ms(2000);
    app_led_on();

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
        while (1) {
            tool_delay_ms(1000);
            app_led_toggle();
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
            tool_io_log_info("Wait for input.");
            input = tool_io_gets(input_buf, APP_INPUT_BUF_SIZE);
        } while (1);
    }
    tool_io_log_warn("System trun off.");
}

/// Wait for input (no more than `capacity-1`) within x mins,
/// otherwise the return value will be `NULL`.
const char* app_get_input_within_x_mins(char* buf, const uint16_t capacity, const uint32_t min) {
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

void app_task_get_info(void) {
    tool_io_log_info("Execute `app_task_get_info`.");
}

void app_task_get_data(void) {
    tool_io_log_info("Execute `app_task_get_data`.");
}