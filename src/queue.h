// queue.h - generic lock-free thread-safe queue
#ifndef QUEUE_H
#define QUEUE_H

#include <stdatomic.h>
#include <stdbool.h>

// Declares types and function signatures
#define DECLARE_QUEUE(T, name, size) \
typedef struct { \
    T buffer[size]; \
    _Atomic int write_idx; \
    _Atomic int read_idx; \
    _Atomic int dropped_count; \
    int capacity; \
} name##_queue_t; \
\
static inline void name##_queue_init(name##_queue_t* q); \
static inline void name##_queue_push(name##_queue_t* q, T v); \
static inline T name##_queue_pop(name##_queue_t* q); \
static inline bool name##_queue_is_empty(name##_queue_t* q);

// Defines function bodies
#define DEFINE_QUEUE(T, name, size, empty_val_expr) \
static inline void name##_queue_init(name##_queue_t* q) { \
    atomic_init(&q->write_idx, 0); \
    atomic_init(&q->read_idx, 0); \
    atomic_init(&q->dropped_count, 0); \
    q->capacity = size; \
} \
\
static inline void name##_queue_push(name##_queue_t* q, T v) { \
    int next = (atomic_load_explicit(&q->write_idx, memory_order_relaxed) + 1) % size; \
    int current_read = atomic_load_explicit(&q->read_idx, memory_order_acquire); \
    if (next == current_read) { \
        atomic_fetch_add(&q->dropped_count, 1); \
        return; \
    } \
    q->buffer[atomic_load_explicit(&q->write_idx, memory_order_relaxed)] = v; \
    atomic_store_explicit(&q->write_idx, next, memory_order_release); \
} \
\
static inline T name##_queue_pop(name##_queue_t* q) { \
    int current_read = atomic_load_explicit(&q->read_idx, memory_order_relaxed); \
    int current_write = atomic_load_explicit(&q->write_idx, memory_order_acquire); \
    if (current_read == current_write) { \
        T empty = empty_val_expr; \
        return empty; \
    } \
    T v = q->buffer[current_read]; \
    atomic_store_explicit(&q->read_idx, (current_read + 1) % size, memory_order_release); \
    return v; \
} \
\
static inline bool name##_queue_is_empty(name##_queue_t* q) { \
    int current_read = atomic_load_explicit(&q->read_idx, memory_order_relaxed); \
    int current_write = atomic_load_explicit(&q->write_idx, memory_order_acquire); \
    return current_read == current_write; \
}

#endif // QUEUE_H
