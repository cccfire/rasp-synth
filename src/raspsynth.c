#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#include "raspsynth.h"

const float global_gain = 0.25;

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
  out_ctx->voices = (raspsynth_voice_t**) calloc(out_ctx->voices_length, sizeof(void*));
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
  raspsynth_voice_t* voice = NULL;
  for (int i = 0; i < ctx->voices_length; i++) {
    if (ctx->voices[i] && ctx->voices[i]->pitch == pitch && ctx->voices[i]->state != RELEASE) {
      if (voice == NULL) {
        voice = ctx->voices[i];
      } else if (ctx->voices[i]->start_time < voice->start_time){
        voice = ctx->voices[i];
      }
    }
  }

  assert(voice != NULL);

  // for polyphony later, go back through and release all the voices with the same 
  //   pitch and timestamp

  for (int i = 0; i < ctx->voices_length; i++) {
    if (ctx->voices[i]
     && ctx->voices[i]->pitch == voice->pitch
     && ctx->voices[i]->start_time == voice->start_time) {
      //
      ctx->voices[i]->state = RELEASE;
      ctx->voices[i]->frame_count = 0;
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
  (void) input; /* Prevent unused variable warning. */

  /*
  float baseFreq = 440.0 * pow(2.0, ((key + pitchNoteExpressionValue + pitchBendWheel +
                                  (oscDetune + oscDetuneMod) / 100) -
                                 69.0) /
                                    12.0);
  */

  
  for(int i = 0; i < frameCount; i++) {
    float voice_gain = ctx->num_voices ? 1.0f / ctx->num_voices : 1.0f;

    // Accumulators for left and right
    float left_acc = 0.0f;
    float right_acc = 0.0f;

    ctx->current_frame++;

    // Loop through voices array and find non-zero voice pointers
    for (int j = 0; j < ctx->voices_length; j++) {
      if (ctx->voices[j]) {
        ctx->voices[j]->frame_count++;
        ctx->voices[j]->step(ctx->voices[j]);
        float out_l = 0.0f;
        float out_r = 0.0f;
        // the voice may no longer exist after process!
        ctx->voices[j]->process(ctx, ctx->voices[j], &out_l, &out_r);

        left_acc += out_l * voice_gain;
        right_acc += out_r * voice_gain;
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
  raspsynth_voice_t** voice_position = NULL;

  // Look for any free spots in the voices array and pick the first free slot
  for (int i = 0; i < ctx->voices_length; i++) {
    if (!ctx->voices[i]) {
      voice_position = &(ctx->voices[i]);
    }
  }

  // allocate space for the voice
  raspsynth_voice_t* voice = (raspsynth_voice_t*) malloc(sizeof(raspsynth_voice_t));

  if (voice_position) {
    *voice_position = voice;
  }
  // If we didn't find any free spots, we will need to expand the voices array
  //   so double the length and realloc. 
  else {
    int length = ctx->voices_length;
    ctx->voices_length = ctx->voices_length * 2;
    ctx->voices = (raspsynth_voice_t**) realloc(ctx->voices, 
      sizeof(raspsynth_voice_t*) * ctx->voices_length);
    // realloc doesn't automatically zero out the memory block, so we must do it ourselves.
    for (int i = length; i < ctx->voices_length; i++) {
      ctx->voices[i] = NULL;
    }
    ctx->voices[length] = voice;
  }


  
  // set voice values
  voice->start_time = time;
  voice->frame_count = 0;
  voice->pitch = pitch;
  voice->velocity = velocity;
  voice->left_phase = 0.0f;
  voice->right_phase = 0.0f;
  voice->oscDetune = 0.0f;
  voice->oscDetuneMod = 0.0f;
  voice->adsr = ctx->amp_adsr;
  voice->state = ATTACK;
  voice->release_level = 0.0f;

  voice->step = raspsynth_step;
  voice->process = raspsynth_sine_process;

  ctx->num_voices++;
}

void raspsynth_remove_voice(raspsynth_ctx_t* ctx, raspsynth_voice_t* voice)
{
  assert(voice);

  // Look for the voice in the voices array.
  for (int i = 0; i < ctx->voices_length; i++) {
    if (ctx->voices[i] == voice)
      ctx->voices[i] = (raspsynth_voice_t*) NULL; // just to be super clear
  }

  // Free voice
  free(voice);

  // decrement num_voices 
  ctx->num_voices--;
}

void raspsynth_step (raspsynth_voice_t* voice)
{
  float baseFreq = 440.0 * pow(2.0, ((voice->pitch +
                                  (voice->oscDetune + voice->oscDetuneMod) / 100) -
                                 69.0) /
                                    12.0);
  voice->left_phase += baseFreq / SAMPLE_RATE;
  voice->right_phase += baseFreq / SAMPLE_RATE;

  voice->left_phase = fmodf(voice->left_phase, 1.0f);
  voice->right_phase = fmodf(voice->right_phase, 1.0f);
}

void raspsynth_process_adsr (raspsynth_ctx_t* ctx, raspsynth_voice_t* voice, float* out_l, float* out_r)
{
  float seconds = ((float) voice->frame_count) / SAMPLE_RATE;
  adsr_ctx_t adsr = voice->adsr;
  float modifier = 0.0f;
  switch (voice->state) {
    case ATTACK:
      modifier = adsr.attack ? fminf(1.0f, seconds / adsr.attack) : 1.0f;
      *out_l = *out_l * modifier;
      *out_r = *out_r * modifier;

      if (seconds > adsr.attack) {
        voice->state = HOLD;
      }
      voice->release_level = modifier;
      break;
    case HOLD:
      voice->release_level = 1.0f;
      modifier = 1.0f;
      *out_l = *out_l * modifier;
      *out_r = *out_r * modifier;
      voice->release_level = modifier;
      if (seconds - adsr.attack > adsr.hold) {
        voice->state = DECAY;
        break;
      }
      break;
    case DECAY:
      if (seconds - adsr.attack - adsr.hold > adsr.decay || adsr.decay == 0.0) {
        voice->state = SUSTAIN;
        modifier = adsr.sustain;
      } else { 
        float t = (seconds - adsr.attack - adsr.hold) / adsr.decay;  // 0.0 to 1.0
        modifier = 1.0f - (t * (1.0f - adsr.sustain));
      }
      
      *out_l = *out_l * modifier;
      *out_r = *out_r * modifier;
      voice->release_level = modifier;


      break;
    case SUSTAIN:
      modifier = adsr.sustain;
      *out_l = *out_l * modifier;
      *out_r = *out_r * modifier;
      voice->release_level = modifier;
      break;
    case RELEASE:
      // RELEASE is set by note_off and when RELEASE is set, the frame_count is set to 0.
      if (seconds >= adsr.release || adsr.release == 0) {
        // We delete the voice.
        raspsynth_remove_voice(ctx, voice);
        *out_l = 0;
        *out_r = 0;
        break;
      }

      modifier = (1.0f - (seconds / adsr.release)) * voice->release_level;
      *out_l = *out_l * modifier;
      *out_r = *out_r * modifier;

      break;
    case OFF:
      raspsynth_remove_voice(ctx, voice);
      break;
  }
  *out_l = fmax(-1.0f, fmin(1.0f, *out_l));  // left 
  *out_r = fmax(-1.0f, fmin(1.0f, *out_r));  // right 
}

void raspsynth_sine_process (raspsynth_ctx_t* ctx, raspsynth_voice_t* voice, float* out_l, float* out_r)
{
  *out_l = sinf(voice->left_phase * 2.0f * M_PI);
  *out_r = sinf(voice->right_phase * 2.0f * M_PI);

  raspsynth_process_adsr(ctx, voice, out_l, out_r);
}
