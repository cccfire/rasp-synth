#ifndef RASPSYNTH_H
#define RASPSYNTH_H

#include <stdbool.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

#include "adsr_screen.h"

typedef struct raspsynth {
  adsr_ctx_t* amp_adsr;
  adsr_ctx_t* filter_adsr;
  float left_phase;
  float right_phase;
} raspsynth_ctx_t;

/**
 * Creates an adsr screen and context.
 *
 * @param[out] out_app  output screen
 * @param[out] out_ctx     output context 
 */
void create_raspsynth(
    cdsl_app_t* out_app, 
    raspsynth_ctx_t* out_ctx);
void raspsynth_init(raspsynth_ctx_t* ctx);
void raspsynth_on_draw(raspsynth_ctx_t* ctx);
void raspsynth_event_callback(const SDL_Event* event, raspsynth_ctx_t* ctx);

int raspsynth_audiogen_callback( 
  const void* input,
  void* output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData);

#endif // RASPSYNTH_H

