#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#include "raspsynth.h"

const float global_gain = 0.025;

const float pival =
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

  out_ctx->current_frame = 0;

  out_ctx->amp_adsr.attack = 0.3;
  out_ctx->amp_adsr.hold = 1.0;
  out_ctx->amp_adsr.decay = 1.0;
  out_ctx->amp_adsr.sustain = 0.7;
  out_ctx->amp_adsr.release = 0.5;

  out_ctx->filter_adsr.attack = 0.3;
  out_ctx->filter_adsr.hold = 0.7;
  out_ctx->filter_adsr.decay = 1.0;
  out_ctx->filter_adsr.sustain = 0.7;
  out_ctx->filter_adsr.release = 0.5;

  // initializes voices to all 0s (because that's how calloc works)
  out_ctx->voices = (voice_t**) calloc(out_ctx->voices_length, sizeof(void*));
}

void destroy_raspsynth(cdsl_app_t* app, raspsynth_ctx_t* ctx)
{
  assert(ctx->voices != NULL && ctx->voices_length != 0);

  // if there are active voices left, iterate through and free them
  for (int i = 0; i < ctx->voices_length; i++) {
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
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint32_t time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

  raspsynth_voice_params_t params;
  raspsynth_start_voice(pitch, velocity, ctx, &params, time);
}

void raspsynth_note_off(int32_t pitch, raspsynth_ctx_t* ctx)
{
  voice_t* voice = NULL;
  for (int i = 0; i < ctx->voices_length; i++) {
    if (ctx->voices[i] 
      && ctx->voices[i]->pitch == pitch 
      && !(ctx->voices[i]->is_released(ctx, ctx->voices[i]))) {

      if (voice == NULL) {
        voice = ctx->voices[i];
      } else if (ctx->voices[i]->start_time < voice->start_time){
        voice = ctx->voices[i];
      }
    }
  }

  // assert(voice != NULL);
  if (!voice) return;

  // for polyphony later, go back through and release all the voices with the same 
  //   pitch and timestamp

  for (int i = 0; i < ctx->voices_length; i++) {
    if (ctx->voices[i]
     && ctx->voices[i]->pitch == voice->pitch
     && ctx->voices[i]->start_time == voice->start_time) {
      //
      printf("release");
      ctx->voices[i]->on_release(ctx, ctx->voices[i]);
    }
  }
}

int raspsynth_audiogen_callback( 
  const void* input,
  void* output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData )
{
  raspsynth_ctx_t* ctx = (raspsynth_ctx_t*) userData; 
  float *out = (float*) output;
  (void) input;

  for(int i = 0; i < frameCount; i++) {

    // Accumulators for left and right
    float left_acc = 0.0f;
    float right_acc = 0.0f;

    ctx->current_frame++;

    // Loop through voices array and find non-zero voice pointers
    for (int j = 0; j < ctx->voices_length; j++) {
      if (ctx->voices[j]) {
        voice_t* voice = ctx->voices[j];
        voice->step(ctx, voice);

        float out_l, out_r;
        voice->process(ctx, voice, &out_l, &out_r);
        adsr_t* adsr = &(((raspsynth_voice_ctx_t*) (voice->ctx))->amp_adsr);
        double amp_mod = process_adsr(adsr, SAMPLE_RATE);

        left_acc += out_l * amp_mod;
        right_acc += out_r * amp_mod;


        if (voice->should_kill(ctx, voice))
          raspsynth_remove_voice(ctx, voice);
      }
    }
    *out++ = fmax(-1.0f, fmin(1.0f, global_gain * left_acc));  // left 
    *out++ = fmax(-1.0f, fmin(1.0f, global_gain * right_acc));  // right 
  }
  return 0;
}

void raspsynth_start_voice(int32_t pitch, int32_t velocity, raspsynth_ctx_t* ctx, 
  raspsynth_voice_params_t* params, uint32_t time)
{
  voice_t** voice_position = NULL;

  // Look for any free spots in the voices array and pick the first free slot
  for (int i = 0; i < ctx->voices_length; i++) {
    if (!ctx->voices[i]) {
      voice_position = &(ctx->voices[i]);
    }
  }

  // allocate space for the voice
  voice_t* voice = (voice_t*) malloc(sizeof(voice_t));

  // allocate space for the voice context
  raspsynth_voice_ctx_t* voice_ctx = (raspsynth_voice_ctx_t*) malloc(sizeof(raspsynth_voice_ctx_t));

  voice->ctx = (void*) voice_ctx;

  if (voice_position) {
    *voice_position = voice;
  }
  
  // If we didn't find any free spots, we will need to expand the voices array
  //   so double the length and realloc. 
  else {
    int length = ctx->voices_length;
    ctx->voices_length = ctx->voices_length * 2;
    ctx->voices = (voice_t**) realloc(ctx->voices, 
      sizeof(voice_t*) * ctx->voices_length);
    // realloc doesn't automatically zero out the memory block, so we must do it ourselves.
    for (int i = length; i < ctx->voices_length; i++) {
      ctx->voices[i] = NULL;
    }
    ctx->voices[length] = voice;
  }

  raspsynth_voice_init (ctx, voice, voice_ctx, pitch, velocity, time);

  ctx->num_voices++;
}

void raspsynth_remove_voice(raspsynth_ctx_t* ctx, voice_t* voice)
{
  assert(voice);

  // Look for the voice in the voices array.
  for (int i = 0; i < ctx->voices_length; i++) {
    if (ctx->voices[i] == voice)
      ctx->voices[i] = (voice_t*) NULL; // just to be super clear
  }

  // Free context
  free(voice->ctx);

  // Free voice
  free(voice);

  // decrement num_voices 
  ctx->num_voices--;
}

void raspsynth_voice_init (void* ctx, voice_t* voice, void* voice_ctx, int32_t pitch, int32_t velocity, uint32_t time)
{
  raspsynth_voice_ctx_t* rv_ctx = (raspsynth_voice_ctx_t*) voice_ctx; 

  rv_ctx->amp_adsr = ((raspsynth_ctx_t*) ctx)->amp_adsr;
  rv_ctx->filter_adsr = ((raspsynth_ctx_t*) ctx)->filter_adsr;

  rv_ctx->amp_adsr.frame_count = 0;
  rv_ctx->filter_adsr.frame_count = 0;

  rv_ctx->amp_adsr.state = ATTACK;
  rv_ctx->filter_adsr.state = ATTACK;
  
  // set voice values
  voice->start_time = time;
  voice->pitch = pitch;
  voice->velocity = velocity;
  voice->left_phase = 0.0f;
  voice->right_phase = 0.0f;
  voice->oscDetune = 0.0f;
  voice->oscDetuneMod = 0.0f;

  voice->step = (void (*) (void*, voice_t*)) raspsynth_voice_step;
  voice->process = (void (*) (void*, voice_t*, float*, float*)) raspsynth_sine_process;
  voice->on_release = (void (*) (void*, voice_t*)) raspsynth_voice_on_release;
  voice->should_kill = (bool (*) (void*, voice_t*)) raspsynth_voice_should_kill;
  voice->is_released = (bool (*) (void*, voice_t*)) raspsynth_voice_is_released;
}

void raspsynth_voice_step (raspsynth_ctx_t* ctx, voice_t* voice)
{
  double baseFreq = 440.0 * pow(2.0, ((voice->pitch +
                                  (voice->oscDetune + voice->oscDetuneMod) / 100) -
                                 69.0) /
                                    12.0);
  voice->left_phase += baseFreq / SAMPLE_RATE;
  voice->right_phase += baseFreq / SAMPLE_RATE;

  voice->left_phase = fmodf(voice->left_phase, 1.0f);
  voice->right_phase = fmodf(voice->right_phase, 1.0f);
}

void raspsynth_sine_process (raspsynth_ctx_t* ctx, voice_t* voice, float* out_l, float* out_r)
{
  *out_l = sinf(voice->left_phase * 2.0f * M_PI);
  *out_r = sinf(voice->right_phase * 2.0f * M_PI);
}

void raspsynth_voice_on_release (raspsynth_ctx_t* app_ctx, voice_t* voice)
{
  raspsynth_voice_ctx_t* ctx = (raspsynth_voice_ctx_t*) (voice->ctx);
  ctx->amp_adsr.state = RELEASE;
  ctx->filter_adsr.state = RELEASE;
  ctx->amp_adsr.frame_count = 0;
  ctx->filter_adsr.frame_count = 0;
}

bool raspsynth_voice_should_kill (raspsynth_ctx_t* ctx, voice_t* voice)
{
  raspsynth_voice_ctx_t* voice_ctx = (raspsynth_voice_ctx_t*) (voice->ctx);
  return (voice_ctx->amp_adsr.state == OFF);
}

bool raspsynth_voice_is_released (raspsynth_ctx_t* app_ctx, voice_t* voice)
{
   return (((raspsynth_voice_ctx_t*)(voice->ctx))->amp_adsr.state == RELEASE);
}

