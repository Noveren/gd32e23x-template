
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
    // app_log_debug("Execute `app_cmd_get_fram_data`.");

    // app_putframe_header_data(app_FRAM_CAPACITY);

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

    // app_putframe_footer();

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

bool dvr_timer_callback(void* _) {
    return dvr_adc_convert_once_async();
}

bool app_cmd_set_adc_timer_start(void) {
    app_log_debug("Execute `app_cmd_set_adc_timer_start`.");
    bool status = true;

    dvr_adc_init(dvr_adc_CHANNEL_0 | dvr_adc_CHANNEL_1 | dvr_adc_CHANNEL_2 | dvr_adc_CHANNEL_3);
    status = dvr_timer_init(dvr_timer_FREQ_100US);
    if (!status) {
        app_print("Failed to init timer.");
        goto app_cmd_set_adc_timer_start_deinit;
    }
    status = dvr_timer_enable(10000);
    if (!status) {
        app_print("Failed to enable timer.");
        goto app_cmd_set_adc_timer_start_deinit;
    }
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
app_cmd_set_adc_timer_start_deinit:
    dvr_timer_deinit();
    dvr_adc_deinit();

    return status;
}

/// CK_SYS = 32MHz, CK_SPI = 16MHz, CK_ADC = 14MHz
/// ADC_DMA = (13.5-10.5us, 7.5-8.7us), SPI = 6.5us, LOGIC = 1.2us
/// 11.79us(正常循环) - 10.0us(循环展开) - 6.41us(寄存器)
bool app_cmd_collect_signal(void) {
    app_log_debug("Execute `app_cmd_collect_signal`.");
    bool status = true;

    dvr_spi_init();
    dvr_adc_init(dvr_adc_CHANNEL_0 | dvr_adc_CHANNEL_1 | dvr_adc_CHANNEL_2 | dvr_adc_CHANNEL_3);
    status = dvr_timer_init(dvr_timer_FREQ_5US);
    if (!status) {
        app_print("Failed to initialize the timer.");
        goto app_cmd_collect_signal_deinit;
    }

    /// 检查铁电存储器是否允许写入
    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_RDSR);
    status = (dvr_spi_access_data(app_FRAM_OPCODE_NOP) & 0x02) != 0;
    dvr_spi_release();
    dvr_spi_disable();
    status = true;
    if (!status) {
        app_print("The FRAM does not allow writing.");
        goto app_cmd_collect_signal_deinit;
    }

    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_WRITE);
    dvr_spi_access_data(0x00);
    dvr_spi_access_data(0x00);
    const volatile uint16_t* result = NULL;
    uint16_t temp[4] = { 0 };
    uint16_t counter = 0;
    status = dvr_timer_enable(2);
    if (!status) {
        app_print("Failed to enable the timer.");
    } else {
        for (;;) {
            if (!dvr_timer_is_working()) {
                app_print("The timer has been disable.");
                break;
            } else {
                if (counter >= 8192) {
                    break;
                } else if ((result = dvr_adc_get_result()) != NULL) {
                    // app_led_toggle(); 
                    counter += 1;
                    /// 缓存结果
                    temp[0] = result[0];
                    temp[1] = result[1];
                    temp[2] = result[2];
                    temp[3] = result[3];
                    result = NULL;
                    // app_led_toggle();
                    /// 存入 FRAM，改为大端先行

                    // for (uint8_t i = 0; i < 4; i++) {
                    //     dvr_spi_access_data(((const uint8_t*)temp)[2*i + 1]);
                    //     dvr_spi_access_data(((const uint8_t*)temp)[2*i    ]);
                    // }

                    // dvr_spi_access_data(((const uint8_t*)temp)[1]);
                    // dvr_spi_access_data(((const uint8_t*)temp)[0]);
                    // dvr_spi_access_data(((const uint8_t*)temp)[3]);
                    // dvr_spi_access_data(((const uint8_t*)temp)[2]);
                    // dvr_spi_access_data(((const uint8_t*)temp)[5]);
                    // dvr_spi_access_data(((const uint8_t*)temp)[4]);
                    // dvr_spi_access_data(((const uint8_t*)temp)[7]);
                    // dvr_spi_access_data(((const uint8_t*)temp)[6]);
                    
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[1];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[0];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[3];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[2];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[5];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[4];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[7];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[6];
                    // app_led_toggle();
                }
            }
        }
        dvr_timer_disable();
    }
    dvr_spi_release();
    dvr_spi_disable();
app_cmd_collect_signal_deinit:
    dvr_timer_deinit();
    dvr_adc_deinit();
    dvr_spi_deinit();
    return status;
}

bool app_cmd_collect_signal_with_triger(void) {
    app_log_debug("Execute `app_cmd_collect_signal_with_triger`.");
    bool status = true;

    const volatile uint16_t* result = NULL;

    dvr_spi_init();
    dvr_adc_init(dvr_adc_CHANNEL_0 | dvr_adc_CHANNEL_1 | dvr_adc_CHANNEL_2 | dvr_adc_CHANNEL_3);
    status = dvr_timer_init(dvr_timer_FREQ_5US);
    if (!status) {
        app_print("Failed to initialize the timer.");
        goto app_cmd_collect_signal_with_triger_deinit;
    }

    /// 检查铁电存储器是否允许写入
    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_RDSR);
    status = (dvr_spi_access_data(app_FRAM_OPCODE_NOP) & 0x02) != 0;
    dvr_spi_release();
    dvr_spi_disable();
    status = true;
    if (!status) {
        app_print("The FRAM does not allow writing.");
        goto app_cmd_collect_signal_with_triger_deinit;
    }

    uint32_t mean_voltage_of_the_first_available_channel = 0;
    const uint8_t shift = 10;
    const uint16_t max_sample_times = 0x1 << shift;
    status = dvr_timer_enable(20);
    if (!status) {
        app_print("Failed to enable the timer.");
        goto app_cmd_collect_signal_with_triger_deinit;
    } else {
        for (uint16_t i = 0; i < max_sample_times;) {
            status = dvr_timer_is_working();
            if (!status) {
                app_print("The timer has been disable.");
                goto app_cmd_collect_signal_with_triger_deinit;
            } else {
                if ((result = dvr_adc_get_result()) != NULL) {
                    i += 1;
                    mean_voltage_of_the_first_available_channel += result[0];
                    result = NULL;
                }
            }
        }
        dvr_timer_disable();
        mean_voltage_of_the_first_available_channel >>= shift;
        app_print("The Mean Voltage (MSB) of the first channel.");
        app_putframe_header_text(8);
        dvr_io_putbytes_text((uint8_t*)&mean_voltage_of_the_first_available_channel, 4, '\x00');
        app_putframe_footer();
    }


    dvr_spi_enable();
    dvr_spi_select();
    dvr_spi_access_data(app_FRAM_OPCODE_WRITE);
    dvr_spi_access_data(0x00);
    dvr_spi_access_data(0x00);
#define RING_BUF_GROUP 512
#define RING_BUF_LENGTH (RING_BUF_GROUP*4)
    uint16_t ring_buf[RING_BUF_LENGTH] = { 0 };
    uint16_t temp[4] = { 0 };
    status = dvr_timer_enable(2);
    if (!status) {
        app_print("Failed to enable the timer.");
        goto app_cmd_collect_signal_with_triger_deinit;
    } else {
        uint16_t ring_buf_idx = 0;
        app_led_on();
        for (;;) {
            status = dvr_timer_is_working();
            if (!status) {
                app_print("The timer has been disable.");
                goto app_cmd_collect_signal_with_triger_deinit;
            } else {
                if ((result = dvr_adc_get_result()) != NULL) {
                    ring_buf[ring_buf_idx + 0] = result[0];
                    ring_buf[ring_buf_idx + 1] = result[1];
                    ring_buf[ring_buf_idx + 2] = result[2];
                    ring_buf[ring_buf_idx + 3] = result[3];
                    ring_buf_idx = ((ring_buf_idx+4) >= RING_BUF_LENGTH) ? 0 : (ring_buf_idx+4);
#define THRESHOLD dvr_adc_0p2V
                    if (   ((mean_voltage_of_the_first_available_channel + THRESHOLD) < result[0])
                        || ((result[0] + THRESHOLD) < mean_voltage_of_the_first_available_channel)
                    ) {
                        break;
                    }
#undef THRESHOLD
                    result = NULL;
                }
            }
        }
        app_led_off();

        for (uint32_t i = 0; i < 8192 - RING_BUF_GROUP;) {
            status = dvr_timer_is_working();
            if (!status) {
                app_print("The timer has been disable.");
                goto app_cmd_collect_signal_with_triger_deinit;
            } else {
                if ((result = dvr_adc_get_result()) != NULL) {
                    i += 1;
                    temp[0] = (result[0] | 0x8000);
                    temp[1] = (result[1] | 0x8000);
                    temp[2] = (result[2] | 0x8000);
                    temp[3] = (result[3] | 0x8000);
                    result = NULL;

                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[1];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[0];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[3];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[2];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[5];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[4];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[7];
                    while (!(RESET != (SPI_STAT(SPI0) & SPI_FLAG_TBE)));
                    SPI_DATA(SPI0) = (uint32_t)((const uint8_t*)temp)[6];
                }
            }
        }
        dvr_timer_disable();

        for (uint32_t i = ring_buf_idx; i < RING_BUF_LENGTH; i += 4) {
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[1]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[0]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[3]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[2]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[5]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[4]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[7]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[6]);
        }
        for (uint32_t i = 0; i < ring_buf_idx; i += 4) {
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[1]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[0]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[3]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[2]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[5]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[4]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[7]);
            dvr_spi_write_data(((uint8_t*)&(ring_buf[i]))[6]);
        }
#undef RING_BUF_LENGTH
#undef RING_BUF_GROUP
    }
    dvr_spi_release();
    dvr_spi_disable();
app_cmd_collect_signal_with_triger_deinit:
    dvr_timer_deinit();
    dvr_adc_deinit();
    dvr_spi_deinit();
    return status;
}

// bool app_cmd_test_timer(void) {
//     app_log_debug("Execute `app_cmd_test_timer`.");
//     bool status = true;
//     status = dvr_timer_init(dvr_timer_FREQ_5US); do {
//         if (!status) {
//             app_print("Failed to init timer.");
//             break;
//         }
//         status = dvr_timer_enable(10);
//         if (!status) {
//             app_print("Failed to enable timer.");
//             break;
//         }
//         for (;;) {
//             if (app_gets_or_NULL() != NULL) {
//                 break;
//             }
//         }
//         dvr_timer_disable();
//     } while (0); dvr_timer_deinit();
//     return true;
// }
