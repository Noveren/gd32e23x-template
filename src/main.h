#ifndef __MAIN_H_
#define __MAIN_H_

#include "gd32e23x.h"
#include "gd32e23x_tool.h"

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

#endif