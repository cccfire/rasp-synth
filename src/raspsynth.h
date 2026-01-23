#ifndef RASPSYNTH_H
#define RASPSYNTH_H

#include <stdbool.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

#include "adsr_screen.h"

typedef struct raspsynth_voice {
  float phase;
} raspsynth_voice_t;

typedef struct raspsynth {
  adsr_ctx_t* amp_adsr;
  adsr_ctx_t* filter_adsr;
  float left_phase;
  float right_phase;
  int num_voices;
  int voices_length;
  raspsynth_voice_t** voices;
} raspsynth_ctx_t;

/**
 * Creates the raspberry pi synth app and populates the context.
 *
 * @param[out] out_app     output app 
 * @param[out] out_ctx     output context 
 */
void create_raspsynth(
    cdsl_app_t* out_app, 
    raspsynth_ctx_t* out_ctx);

/**
 * Does any cleanup needed.
 * Frees ctx->voices and replaces it with a null pointer.
 *
 * @param[in] app  input app
 * @param[in] ctx  input ctx
 */
void destroy_raspsynth(
    cdsl_app_t* app, 
    raspsynth_ctx_t* ctx);

void raspsynth_init(raspsynth_ctx_t* ctx);
void raspsynth_on_draw(raspsynth_ctx_t* ctx);
void raspsynth_event_callback(const SDL_Event* event, raspsynth_ctx_t* ctx);

void raspsynth_add_voice(raspsynth_ctx_t* out_ctx);

int raspsynth_audiogen_callback( 
  const void* input,
  void* output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData);

#endif // RASPSYNTH_H

