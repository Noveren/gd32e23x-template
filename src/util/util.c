
#include "util.h"

void util_ringq_init(util_ringq_t* q, uint16_t capacity, volatile uint8_t* memory) {
    q->head = 0;
    q->tail = 0;
    q->flag = 0;
    q->capacity = capacity;
    q->memory = memory;
}

int util_ringq_push(util_ringq_t* q, uint8_t byte) {
    if (util_ringq_is_full(q)) {
        return -1;
    }
    q->memory[q->tail] = byte;
    q->tail = (q->tail + 1) % q->capacity;
    if (q->tail == q->head) {
        q->flag = 1;
    }
    return 0;
}

int util_ringq_poll(util_ringq_t* q) {
    if (util_ringq_is_empty(q)) {
        return -1;
    }
    int ret = (int)(q->memory[q->head]);
    q->head = (q->head + 1) % q->capacity;
    if (q->tail == q->head) {
        q->flag = 0;
    }
    return ret;
}

uint16_t util_byte2ascii(uint8_t byte) {
    uint16_t ret = 0x0000;
    const uint8_t high = (byte & 0xF0) >> 4;
    const uint8_t low  = (byte & 0x0F)     ;
    ret |= (((uint16_t)(high + (high <= 0x09 ? 0x30 : 0x37))) << 8);
    ret |= (((uint16_t)(low  + (low  <= 0x09 ? 0x30 : 0x37)))     );
    return ret;
}

uint32_t cstrlen(const char* cstr) {
    uint32_t counter = 0;
    while (counter < util_CSTR_MAX_LENGTH && *cstr) {
        cstr += 1;
        counter += 1;
    }
    return counter;
}

int32_t cstrcmp(const char* cstr1, const char* cstr2) {
    uint16_t counter = 0;
    while (counter < util_CSTR_MAX_LENGTH && (*cstr1 == *cstr2) && *cstr1 && *cstr2) {
        cstr1 += 1;
        cstr2 += 1;
        counter += 1;
    }
    const int32_t flag = (int32_t)*cstr1 - (int32_t)*cstr2;
    if (flag == 0) {
        return 0;
    } else {
        return flag > 0 ? (counter) : (-counter);
    }
}