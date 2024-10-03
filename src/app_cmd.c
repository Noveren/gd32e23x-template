
#include "app.h"
#include "driver.h"

bool app_cmd_get_fram_id(void) {
    app_log_debug("Execute `app_cmd_get_fram_id`.");

    app_FRAM_ID id = { 0 };

    dvr_spi_init();
    dvr_spi_enable();
    dvr_spi_select();

    dvr_spi_access_data(app_FRAM_OPCODE_RDID);
    for (int i = 0; i < 9; i++) {
        ((uint8_t*)(&id))[i] = dvr_spi_access_data(app_FRAM_OPCODE_NOP);
    }

    dvr_spi_release();
    dvr_spi_disable();
    dvr_spi_deinit();

    app_putframe_header_text(18);
    dvr_io_putbytes_text((uint8_t*)&id, 9, '\x00');
    app_putframe_footer();

    return true;
}

bool app_cmd_get_fram_status_register(void) {
    app_log_debug("Execute `app_cmd_get_fram_status_register`.");

    uint8_t reg = 0;

    dvr_spi_init();
    dvr_spi_enable();
    dvr_spi_select();

    dvr_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = dvr_spi_access_data(app_FRAM_OPCODE_NOP);

    dvr_spi_release();
    dvr_spi_disable();
    dvr_spi_deinit();

    app_putframe_header_text(1);
    dvr_io_putbytes_text(&reg, 1, '\x00');
    app_putframe_footer();

    return true;
}

bool app_cmd_get_fram_data(void) {
    app_log_debug("Execute `app_cmd_get_fram_data`.");

    app_putframe_header_data(app_FRAM_CAPACITY);

    dvr_spi_init();
    dvr_spi_enable();
    dvr_spi_select();

    dvr_spi_access_data(app_FRAM_OPCODE_READ);
    dvr_spi_access_data(0x00);
    dvr_spi_access_data(0x00);
    uint8_t byte = 0;
    for (uint32_t i = 0; i < app_FRAM_CAPACITY; i++) {
        byte = dvr_spi_access_data(app_FRAM_OPCODE_NOP);
        dvr_io_putbyte(byte);
    }

    dvr_spi_release();
    dvr_spi_disable();
    dvr_spi_deinit();

    app_putframe_footer();

    return true;
}

bool app_cmd_set_fram_write_enable(void) {
    app_log_debug("Execute `app_cmd_set_fram_write_enable`.");

    dvr_spi_init();

    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_WREN);
    dvr_spi_release();
    dvr_spi_disable();

    dvr_delay_ms(1);

    uint8_t reg = 0;
    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = dvr_spi_access_data(app_FRAM_OPCODE_NOP);
    dvr_spi_release();
    dvr_spi_disable();

    dvr_spi_deinit();

    return (reg & 0x02) != 0;
}

bool app_cmd_set_fram_clean(void) {
    app_log_debug("Execute `app_cmd_set_fram_clean`.");
    bool ret = false;

    dvr_spi_init();

    uint8_t reg = 0;
    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = dvr_spi_access_data(app_FRAM_OPCODE_NOP);
    dvr_spi_release();
    dvr_spi_disable();

    if ((reg & 0x02) == 0) {
        ret = false;
    } else {
        dvr_spi_enable();
        dvr_spi_select();
        dvr_spi_access_data(app_FRAM_OPCODE_WRITE);
        dvr_spi_access_data(0x00);
        dvr_spi_access_data(0x00);
        for (uint32_t i = 0; i < app_FRAM_CAPACITY; i++) {
            dvr_spi_access_data(0xFF);
        }
        dvr_spi_release();
        dvr_spi_disable();
        ret = true;
    }

    dvr_spi_deinit();

    return ret;
}

bool app_cmd_get_adc_once(void) {
    app_log_debug("Execute `app_cmd_get_adc_once`.");
    bool ret = false;

    dvr_adc_init(dvr_adc_CHANNEL_0 | dvr_adc_CHANNEL_1 | dvr_adc_CHANNEL_2 | dvr_adc_CHANNEL_3);

    if (dvr_adc_convert_once_async()) {
        const volatile uint16_t* result = NULL;
        for (;;) {
            if ((result = dvr_adc_get_result()) != NULL) {
                app_putframe_header_text(24);
                dvr_io_putbytes_text((const uint8_t *)result, 8, ' ');
                app_putframe_footer();
                break;
            }
        }
        ret = true;
    } else {
        ret = false;
    }

    dvr_adc_deinit();

    return ret;
}

static bool app_timer_callbackfn(void* _) {
    return dvr_adc_convert_once_async();
}

bool app_cmd_set_adc_timer_start(void) {
    app_log_debug("Execute `app_cmd_set_adc_timer_start`.");

    dvr_adc_init(dvr_adc_CHANNEL_0 | dvr_adc_CHANNEL_1 | dvr_adc_CHANNEL_2 | dvr_adc_CHANNEL_3);

    dvr_timer_init(dvr_timer_FREQ_100US);
    dvr_timer_enable(10000, app_timer_callbackfn);
    const volatile uint16_t* result = NULL;
    for (;;) {
        if (app_gets_or_NULL() != NULL) {
            break;
        }
        if ((result = dvr_adc_get_result()) != NULL) {
            app_putframe_header_text(24);
            dvr_io_putbytes_text((const uint8_t *)result, 8, ' ');
            app_putframe_footer();
        }
    }
    dvr_timer_disable();
    dvr_timer_deinit();

    dvr_adc_deinit();

    return true;
}

bool app_cmd_collect_signal(void) {
    app_log_debug("Execute `app_cmd_collect_signal`.");
    bool ret = false;

    dvr_spi_init();

    uint8_t reg = 0;
    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_RDSR);
    reg = dvr_spi_access_data(app_FRAM_OPCODE_NOP);
    dvr_spi_release();
    dvr_spi_disable();
    if ((reg & 0x02) == 0) {
        ret = false;
    } else {
        dvr_spi_enable();
        dvr_spi_select();
        dvr_spi_access_data(app_FRAM_OPCODE_WRITE);
        dvr_spi_access_data(0x00);
        dvr_spi_access_data(0x00);

        dvr_adc_init(dvr_adc_CHANNEL_0 | dvr_adc_CHANNEL_1 | dvr_adc_CHANNEL_2 | dvr_adc_CHANNEL_3);

        const volatile uint16_t* result = NULL; /// volatile 防止编译器优化
        uint16_t result_buf[4] = { 0 };
        uint16_t counter = 0;
        dvr_timer_init(dvr_timer_FREQ_10US);
        dvr_timer_enable(1, app_timer_callbackfn);
        for (;;) {
            // if (app_gets_or_NULL() != NULL) {
            //     break;
            // }
            if (counter >= 8192 || !dvr_timer_is_working()) {
                break;
            }
            if ((result = dvr_adc_get_result()) != NULL) {
                counter += 1;
                /// 缓存结果
                result_buf[0] = result[0];
                result_buf[1] = result[1];
                result_buf[2] = result[2];
                result_buf[3] = result[3];
                /// 存入 FRAM
                for (uint8_t i = 0; i < 8; i++) {
                    dvr_spi_access_data(((const uint8_t*)result_buf)[i]);
                }
            }
        }
        dvr_timer_disable();
        dvr_timer_deinit();

        dvr_spi_release();
        dvr_spi_disable();

        dvr_adc_deinit();
        ret = true;
    }
    dvr_spi_deinit();

    return ret;
}


