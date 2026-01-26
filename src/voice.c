#include <stdatomic.h>
#include <math.h>

#include "voice.h"

void sine_process (void* ctx, voice_t* voice, float* out_l, float* out_r) {
  *out_l = sinf(voice->left_phase * 2.0f * M_PI);
  *out_r = sinf(voice->right_phase * 2.0f * M_PI);
}
