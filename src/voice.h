#ifndef VOICE_H
#define VOICE_H

#include <stdbool.h>
#include <stdatomic.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

#include "adsr.h"

#define QUEUE_SIZE (64)

typedef struct voice voice_t;

typedef struct voice {
  uint32_t start_time;
  int32_t pitch;
  int32_t velocity;
  double left_phase;
  double right_phase;
  double oscDetune;
  double oscDetuneMod;
  void* ctx;

  void (*step) (void* app_ctx, voice_t* voice);
  void (*process) (void* app_ctx, voice_t* voice, float* out_l, float* out_r);
  void (*on_release) (void* app_ctx, voice_t* voice);
  bool (*should_kill) (void* app_ctx, voice_t* voice);
  bool (*is_released) (void* app_ctx, voice_t* voice);
} voice_t;

typedef struct {
  voice_t* buffer[QUEUE_SIZE];
  _Atomic int write_idx;
  _Atomic int read_idx;

  // Purely for debugging purposes:
  _Atomic int dropped_count;  // Track failures
} voice_queue_t;


void voice_queue_push(voice_queue_t* q, voice_t* v);
voice_t* voice_queue_pop(voice_queue_t* q);

/**
 * process_adsr does most of the work but do need to make sure the envelopes are released when
 * voice is turned off.
 */
void sine_process (void* ctx, voice_t* voice, float* out_l, float* out_r);
#endif // VOICE_H
