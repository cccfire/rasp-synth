#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#include "raspsynth.h"

float pival =
    3.14159265358979323846;

void create_raspsynth(cdsl_app_t* out_app, raspsynth_ctx_t* out_ctx)
{
  out_app->init = (void (*) (void*)) raspsynth_init;
  out_app->on_draw = (void (*) (void*)) raspsynth_on_draw;
  out_app->event_callback = (void (*) (const SDL_Event*, void*)) raspsynth_event_callback;
  out_app->audiogen_callback = raspsynth_audiogen_callback;
  out_app->note_on = (void (*) (const int32_t, const int32_t, void* )) raspsynth_note_on;
  out_app->note_off = (void (*) (const int32_t, void*)) raspsynth_note_off;

  out_ctx->left_phase = 0.0;
  out_ctx->right_phase = 0.0;

  out_ctx->num_voices = 0;
  out_ctx->voices_length = 10;
    
  out_ctx->voices = (raspsynth_voice_t**)calloc(out_ctx->voices_length, sizeof(void*));
}

void destroy_raspsynth(cdsl_app_t* app, raspsynth_ctx_t* ctx)
{
  assert(ctx->voices != NULL && ctx->voices_length != 0);

  // if there are active voices left, iterate through and free them
  for (int i = 0; i < ctx->num_voices; i++) {
    free(ctx->voices[i]);
  }

  ctx->num_voices = 0;
  ctx->voices_length = 0;
  free(ctx->voices);
  ctx->voices = NULL;
}

void raspsynth_init(raspsynth_ctx_t* ctx)
{
}

void raspsynth_on_draw(raspsynth_ctx_t* ctx)
{
}

void raspsynth_event_callback(const SDL_Event* event, raspsynth_ctx_t* ctx)
{
}

void raspsynth_note_on(int32_t pitch, int32_t velocity, raspsynth_ctx_t* ctx)
{
  raspsynth_voice_t* voice = NULL;



  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  voice->start_time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void raspsynth_note_off(int32_t pitch, raspsynth_ctx_t* ctx)
{
  raspsynth_voice_t* voice = NULL;
  for (int i = 0; i < ctx->num_voices; i++) {
    if (ctx->voices[i]->pitch == pitch) {
      if (voice == 0) {
        voice = ctx->voices[i];
      } else if (ctx->voices[i]->start_time < voice->start_time){
        voice = ctx->voices[i];
      }
    }
  }

  assert(voice != NULL);
  voice->state = RELEASE;
  voice->filter_time = 0;
}

int raspsynth_audiogen_callback( 
  const void* input,
  void* output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData )
{
  raspsynth_ctx_t* data = (raspsynth_ctx_t*) userData; 
  float *out = (float*) output;
  unsigned int i;
  (void) input; /* Prevent unused variable warning. */

  /*
  float baseFreq = 440.0 * pow(2.0, ((key + pitchNoteExpressionValue + pitchBendWheel +
                                  (oscDetune + oscDetuneMod) / 100) -
                                 69.0) /
                                    12.0);
  */
  
  for( i = 0; i < frameCount; i++ )
  {
    *out++ = data->left_phase;  /* left */
    *out++ = data->right_phase;  /* right */
    /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
    data->left_phase += 0.01f;
    /* When signal reaches top, drop back down. */
    if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
    /* higher pitch so we can distinguish left and right. */
    data->right_phase += 0.03f;
    if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
  }
  return 0;
}

