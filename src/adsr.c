#include <stdio.h>
#include <math.h>
#include "adsr.h"

double process_adsr (adsr_t* adsr, int sample_rate)
{
  adsr->frame_count++;
  double seconds = ((double) adsr->frame_count) / sample_rate;
  double out = 1.0f;
  double modifier = 0.0f;

  if (adsr->state == ATTACK) {
    modifier = adsr->attack ? fminf(1.0f, seconds / adsr->attack) : 1.0f;
    out = out * modifier;

    if (seconds >= adsr->attack) {
      adsr->state = HOLD;
    }
    adsr->release_level = modifier;
  }
  if (adsr->state == HOLD) {
    adsr->release_level = 1.0f;
    modifier = 1.0f;
    out = out * modifier;
    adsr->release_level = modifier;
    if (seconds - adsr->attack > adsr->hold) {
      adsr->state = DECAY;
    }
  }
  if (adsr->state == DECAY) {
    if (seconds - adsr->attack - adsr->hold > adsr->decay || adsr->decay == 0.0) {
      adsr->state = SUSTAIN;
    } else { 
      double t = (seconds - adsr->attack - adsr->hold) / adsr->decay;  // 0.0 to 1.0
      modifier = 1.0f - (t * (1.0f - adsr->sustain));
      out = out * modifier;
      adsr->release_level = modifier;
    }
  }
  if (adsr->state == SUSTAIN) {
    modifier = adsr->sustain;
    out = out * modifier;
    adsr->release_level = modifier;
  }
  if (adsr->state == RELEASE) {
    // RELEASE is set by note_off and when RELEASE is set, the frame_count is set to 0.
    if (seconds >= adsr->release || adsr->release == 0) {
      adsr->state = OFF;
      
      out = 0;
    } else {
      modifier = (1.0f - (seconds / adsr->release)) * adsr->release_level;
      out = out * modifier;
    }
  }
  if (adsr->state == OFF) {
    out = 0.0;
  }
  
  return fmax(-1.0f, fmin(1.0f, out));  
}

