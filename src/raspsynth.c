#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#include "raspsynth.h"

const float global_gain = 0.025;

const float pival =
    3.14159265358979323846;

void create_raspsynth(cdsl_app_t* out_app, raspsynth_ctx_t* out_ctx, uint16_t max_voices)
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

  bool* voice_active = (bool*) calloc(max_voices, sizeof(bool));
  int* active_voice_list = (int*) calloc(max_voices, sizeof(int));
  voice_t* voices = (voice_t*) calloc(max_voices, sizeof(voice_t));
  raspsynth_voice_ctx_t* voice_contexts = (raspsynth_voice_ctx_t*) calloc(max_voices, sizeof(raspsynth_voice_ctx_t));

  assert (voice_active);
  assert (active_voice_list);
  assert (voices);
  assert (voice_contexts);

  out_ctx->max_voices = max_voices;

  out_ctx->voice_active = voice_active;
  out_ctx->active_voice_list = active_voice_list;
  out_ctx->voices = voices;
  out_ctx->voice_contexts = voice_contexts;

  out_ctx->current_frame = 0;

  out_ctx->amp_adsr.attack = 0.5;
  out_ctx->amp_adsr.hold = 1.0;
  out_ctx->amp_adsr.decay = 1.0;
  out_ctx->amp_adsr.sustain = 0.0;
  out_ctx->amp_adsr.release = 0.5;

  out_ctx->filter_adsr.attack = 0.3;
  out_ctx->filter_adsr.hold = 0.7;
  out_ctx->filter_adsr.decay = 1.0;
  out_ctx->filter_adsr.sustain = 0.7;
  out_ctx->filter_adsr.release = 0.5;

  atomic_init(&out_ctx->voice_events.write_idx, 0);
  atomic_init(&out_ctx->voice_events.read_idx, 0);
  atomic_init(&out_ctx->voice_events.dropped_count, 0);

  atomic_init(&out_ctx->dropped_voices, 0);
}

void destroy_raspsynth(cdsl_app_t* app, raspsynth_ctx_t* ctx)
{
  ctx->num_voices = 0;
  ctx->max_voices = 0;
  free(ctx->voices);
  free(ctx->voice_active);
  free(ctx->voice_contexts);
  ctx->voices = NULL;
  ctx->voice_active = NULL;
  ctx->voice_contexts = NULL;
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

  voice_event_t event = {
    .timestamp = time,
    .pitch = pitch,
    .velocity = velocity,
    .type = VOICE_EVENT_START
  };

  voice_queue_push(&ctx->voice_events, event);
}

void raspsynth_note_off(int32_t pitch, raspsynth_ctx_t* ctx)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint32_t time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

  voice_event_t event = {
    .timestamp = time,
    .pitch = pitch,
    .velocity = 0.0f,
    .type = VOICE_EVENT_RELEASE
  };
  
  voice_queue_push(&ctx->voice_events, event);
}

void raspsynth_release_note(raspsynth_ctx_t* ctx, int32_t pitch)
{
  voice_t* voice = NULL;
  for (int i = 0; i < ctx->num_voices; i++) {
    int idx = ctx->active_voice_list[i];
    assert (ctx->voice_active[idx]);
    if (ctx->voices[idx].pitch == pitch 
      && !(ctx->voices[idx].is_released(ctx, &ctx->voices[idx]))) {
      if (voice == NULL) {
        voice = &ctx->voices[idx];
      } else if (ctx->voices[idx].start_time < voice->start_time){
        voice = &ctx->voices[idx];
      }
    }
  }


  // assert(voice != NULL);
  if (!voice) {
    return;
  }

  // for polyphony later, go back through and release all the voices with the same 
  //   pitch and timestamp

  for (int i = 0; i < ctx->num_voices; i++) {
    int idx = ctx->active_voice_list[i];
    if (ctx->voice_active[idx]
     && ctx->voices[idx].pitch == voice->pitch
     && ctx->voices[idx].start_time == voice->start_time) {
      //
      ctx->voices[idx].on_release(ctx, &ctx->voices[idx]);
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

  // Process voice event queues.
  raspsynth_process_event_queue(ctx);

  for(int i = 0; i < frameCount; i++) {

    // Accumulators for left and right
    float left_acc = 0.0f;
    float right_acc = 0.0f;

    ctx->current_frame++;

    // Loop through voices array and find non-zero voice pointers
    for (int j = ctx->num_voices-1; j >= 0; j--) {
      int idx = ctx->active_voice_list[j];
      assert (ctx->voice_active[idx]);
      if (ctx->voice_active[idx]) {
        assert(ctx->voices[idx].step != NULL);
        assert(ctx->voices[idx].process != NULL);
        assert(ctx->voices[idx].should_kill != NULL);
        ctx->voices[idx].step(ctx, &ctx->voices[idx]);

        float out_l, out_r;
        ctx->voices[idx].process(ctx, &ctx->voices[idx], &out_l, &out_r);

        adsr_t* adsr = &(((raspsynth_voice_ctx_t*) (ctx->voices[idx].ctx))->amp_adsr);
        double amp_mod = process_adsr(adsr, SAMPLE_RATE);

        ctx->voices[idx].current_left = out_l * amp_mod;
        ctx->voices[idx].current_right = out_l * amp_mod;

        left_acc += ctx->voices[idx].current_left;
        right_acc += ctx->voices[idx].current_right;

        if (ctx->voices[idx].should_kill(ctx, &ctx->voices[idx])) {
          assert (ctx->voices[idx].current_left == 0);
          assert (ctx->voices[idx].current_right == 0);
          raspsynth_remove_voice(ctx, &ctx->voices[idx]);
        }

      }
    }

    *out++ = fmax(-1.0f, fmin(1.0f, global_gain * left_acc));  // left 
    *out++ = fmax(-1.0f, fmin(1.0f, global_gain * right_acc));  // right 
  }
  return 0;
}

void raspsynth_process_event_queue(raspsynth_ctx_t* ctx) 
{
  voice_event_t event;

  while ((event = voice_queue_pop(&ctx->voice_events)).type != VOICE_EVENT_END) {
    if (event.type == VOICE_EVENT_START) {
      voice_t voice = raspsynth_create_default_voice(ctx, event.pitch, event.velocity, event.timestamp);
      raspsynth_voice_ctx_t voice_ctx = raspsynth_create_default_voice_ctx(ctx);
      raspsynth_start_voice(ctx, voice, voice_ctx);
    } else if (event.type == VOICE_EVENT_RELEASE) {
      raspsynth_release_note(ctx, event.pitch);
    }
  }
}

void raspsynth_start_voice(raspsynth_ctx_t* ctx, voice_t voice, raspsynth_voice_ctx_t voice_ctx)
{
  if(ctx->num_voices == ctx->max_voices) {
    atomic_fetch_add(&ctx->dropped_voices, 1);
    return;
  } 

  // Look for any free spots in the voices array and pick the first free slot
  for (int i = 0; i < ctx->max_voices; i++) {
    if (!ctx->voice_active[i]) {
      voice.ctx = &ctx->voice_contexts[i];
      ctx->voice_contexts[i] = voice_ctx;
      ctx->voices[i] = voice;
      ctx->voice_active[i] = true;
      ctx->active_voice_list[ctx->num_voices] = i;
      ctx->num_voices++;
      break;
    }
  }
}

void raspsynth_remove_voice(raspsynth_ctx_t* ctx, voice_t* voice)
{
  assert(voice);

  if (ctx->num_voices == 0) return;

  // Look for the voice in the voices array.
  int voice_idx = (int32_t)(voice - ctx->voices);
  ctx->voice_active[voice_idx] = false;


  // Look for the voice in the active_voices_list
  for (int i = 0; i < ctx->num_voices; i++) {
    if (ctx->active_voice_list[i] == voice_idx) {
      // Replace hole with last entry in list
      ctx->active_voice_list[i] = ctx->active_voice_list[ctx->num_voices - 1];
      break;
    }
  }

  // decrement num_voices 
  ctx->num_voices--;

  // invariant. might break if queue processing gets parallelized. fine for now. 
  assert(ctx->num_voices == 0 || ctx->voice_active[ctx->active_voice_list[ctx->num_voices-1]]);
}

voice_t raspsynth_create_default_voice (raspsynth_ctx_t* ctx, int32_t pitch, int32_t velocity, uint32_t time)
{
  voice_t voice;

  // set voice values
  voice.start_time = time;
  voice.pitch = pitch;
  voice.velocity = velocity;
  voice.current_left = 0.0f;
  voice.current_right = 0.0f;
  voice.left_phase = 0.0f;
  voice.right_phase = 0.0f;
  voice.oscDetune = 0.0f;
  voice.oscDetuneMod = 0.0f;
  voice.ctx = NULL;

  voice.step = (void (*) (void*, voice_t*)) raspsynth_voice_step;
  voice.process = (void (*) (void*, voice_t*, float*, float*)) raspsynth_sine_process;
  voice.on_release = (void (*) (void*, voice_t*)) raspsynth_voice_on_release;
  voice.should_kill = (bool (*) (void*, voice_t*)) raspsynth_voice_should_kill;
  voice.is_released = (bool (*) (void*, voice_t*)) raspsynth_voice_is_released;

  return voice;
}

raspsynth_voice_ctx_t raspsynth_create_default_voice_ctx (raspsynth_ctx_t* ctx)
{
  raspsynth_voice_ctx_t rv_ctx; 

  rv_ctx.amp_adsr = ((raspsynth_ctx_t*) ctx)->amp_adsr;
  rv_ctx.filter_adsr = ((raspsynth_ctx_t*) ctx)->filter_adsr;

  rv_ctx.amp_adsr.frame_count = 0;
  rv_ctx.filter_adsr.frame_count = 0;

  rv_ctx.amp_adsr.state = ATTACK;
  rv_ctx.filter_adsr.state = ATTACK;

  return rv_ctx;
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
  sine_process((void*) ctx, voice, out_l, out_r);
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
  raspsynth_voice_ctx_t* voice_ctx = (raspsynth_voice_ctx_t*) (voice->ctx);
  return (voice_ctx->amp_adsr.state == RELEASE);
}

