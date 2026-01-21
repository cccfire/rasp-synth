#include "raspsynth.h"

void create_raspsynth(cdsl_app_t* out_app, raspsynth_ctx_t* out_ctx)
{
  out_app->init = (void (*) (void*)) raspsynth_init;
  out_app->on_draw = (void (*) (void*)) raspsynth_on_draw;
  out_app->event_callback = (void (*) (const SDL_Event*, void*)) raspsynth_event_callback;
  out_app->audiogen_callback = raspsynth_audiogen_callback;

  out_ctx->left_phase = 0.0;
  out_ctx->left_phase = 0.0;
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

