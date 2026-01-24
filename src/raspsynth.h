#ifndef RASPSYNTH_H
#define RASPSYNTH_H

#include <stdbool.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

#include "adsr.h"
#include "voice.h"
#include "app.h"

#define SAMPLE_RATE (44100)

typedef struct raspsynth raspsynth_ctx_t;


typedef struct raspsynth_voice_ctx {
  adsr_t amp_adsr;
  adsr_t filter_adsr;
} raspsynth_voice_ctx_t;

typedef struct raspsynth_voice_params {
} raspsynth_voice_params_t;

typedef struct raspsynth {
  int sample_rate;
  adsr_t amp_adsr;
  adsr_t filter_adsr;
  float left_phase;
  float right_phase;
  int num_voices;
  int voices_length;
  uint64_t current_frame;
  voice_t** voices;
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

void raspsynth_note_on(const int32_t pitch, const int32_t velocity, raspsynth_ctx_t* ctx);
void raspsynth_note_off(const int32_t pitch, raspsynth_ctx_t* ctx);

int raspsynth_audiogen_callback( 
  const void* input,
  void* output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData);

void raspsynth_start_voice(int32_t pitch, int32_t velocity, raspsynth_ctx_t* ctx, 
    raspsynth_voice_params_t* params, uint32_t time);

void raspsynth_remove_voice(raspsynth_ctx_t* ctx, voice_t* voice);

void raspsynth_voice_init (void* ctx, voice_t* voice, void* voice_ctx, int32_t pitch, int32_t velocity, uint32_t time);

void raspsynth_voice_step (raspsynth_ctx_t* ctx, voice_t* voice); 

void raspsynth_sine_process (raspsynth_ctx_t* ctx, voice_t* voice, float* out_l, float* out_r);

void raspsynth_voice_on_release (raspsynth_ctx_t* ctx, voice_t* voice); 
bool raspsynth_voice_should_kill (raspsynth_ctx_t* app_ctx, voice_t* voice);
bool raspsynth_voice_is_released (raspsynth_ctx_t* ctx, voice_t* voice); 
#endif // RASPSYNTH_H

