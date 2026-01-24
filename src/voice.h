#ifndef VOICE_H
#define VOICE_H

#include <stdbool.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

#include "adsr.h"

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

/**
* process_adsr does most of the work but do need to make sure the envelopes are released when
* voice is turned off.
*/
void sine_process (void* ctx, voice_t* voice, float* out_l, float* out_r);
#endif // VOICE_H
