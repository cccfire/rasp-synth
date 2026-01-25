#include <stdatomic.h>
#include <math.h>

#include "voice.h"

void voice_queue_push(voice_event_queue_t* q, voice_event_t v) 
{
  int next = (atomic_load_explicit(&q->write_idx, memory_order_relaxed) + 1) % QUEUE_SIZE;
  int current_read = atomic_load_explicit(&q->read_idx, memory_order_acquire);

  if (next == current_read) {
    atomic_fetch_add(&q->dropped_count, 1);
    return;
  }

  q->buffer[atomic_load_explicit(&q->write_idx, memory_order_relaxed)] = v;
  atomic_store_explicit(&q->write_idx, next, memory_order_release);
}

voice_event_t voice_queue_pop(voice_event_queue_t* q) 
{
  int current_read = atomic_load_explicit(&q->read_idx, memory_order_relaxed);
  int current_write = atomic_load_explicit(&q->write_idx, memory_order_acquire);

  if (current_read == current_write) {
    voice_event_t ev = {
      .type = VOICE_EVENT_END
    };
    return ev;
  }

  voice_event_t v = q->buffer[current_read];
  atomic_store_explicit(&q->read_idx, (current_read + 1) % QUEUE_SIZE, memory_order_release);
  return v;
}

void sine_process (void* ctx, voice_t* voice, float* out_l, float* out_r) {
  *out_l = sinf(voice->left_phase * 2.0f * M_PI);
  *out_r = sinf(voice->right_phase * 2.0f * M_PI);
}
