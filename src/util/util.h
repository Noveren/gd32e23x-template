#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "cmsis_compiler.h"

/// The behavior of string functions is undefined,
/// if the string length exceeds `util_CSTR_MAX_LENGTH`.
#define util_CSTR_MAX_LENGTH 512

typedef bool (*CallbackFn)(void *);

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint16_t flag;
    uint16_t capacity;
    volatile uint8_t* memory;
} util_ringq_t;

__STATIC_FORCEINLINE bool util_ringq_is_empty(util_ringq_t* q) { return (q->head == q->tail) && q->flag == 0; }
__STATIC_FORCEINLINE bool util_ringq_is_full(util_ringq_t* q)  { return (q->head == q->tail) && q->flag != 0; }

extern void util_ringq_init(util_ringq_t* q, uint16_t capacity, volatile uint8_t* memory);
extern int util_ringq_push(util_ringq_t* q, uint8_t byte);
extern int util_ringq_poll(util_ringq_t* q);

uint16_t util_byte2ascii(uint8_t byte);

uint32_t cstrlen(const char* cstr);
int32_t cstrcmp(const char* cstr1, const char* cstr2);

#endif