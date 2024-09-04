
#include "ringq.h"

void ringq_init(RingQ* q, uint32_t size, uint8_t* space) {
    q->head = 0;
    q->tail = 0;
    q->flag = 0;
    q->size = size;
    q->space = space;
}

/// 返回值 1: 队列已满，入队失败
/// 返回值 0: 入队成功
uint32_t ringq_push(RingQ* q, uint8_t data) {
    if (ringq_is_full(q)) {
        return 1;
    }
    q->space[q->tail] = data;
    q->tail = (q->tail + 1) % q->size;
    if (q->tail == q->head) {
        q->flag = 1;
    }
    return 0;
}

/// 返回值 1：队列为空，出队失败
/// 返回值 0：出队成功
uint32_t ringq_poll(RingQ* q, uint8_t* out) {
    if (ringq_is_empty(q)) {
        return 1;
    }
    *out = q->space[q->head];
    q->head = (q->head + 1) % q->size;
    if (q->tail == q->head) {
        q->flag = 0;
    }
    return 0;
}