#ifndef __RINGQ_H_
#define __RINGQ_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t flag;
    uint32_t size;
    uint8_t* space;
} RingQ;

#define ringq_is_empty(ringq_ptr) (((ringq_ptr)->head == (ringq_ptr)->tail) && ((ringq_ptr)->flag == 0))
#define ringq_is_full(ringq_ptr)  (((ringq_ptr)->head == (ringq_ptr)->tail) && ((ringq_ptr)->flag != 0))

void ringq_init(RingQ* q, uint32_t size, uint8_t* space);
uint32_t ringq_push(RingQ* q, uint8_t data);
uint32_t ringq_poll(RingQ* q, uint8_t* out);

#endif