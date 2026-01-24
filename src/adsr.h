#ifndef ADSR_H
#define ADSR_H

#include <stdbool.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

typedef enum envelope_state {
  OFF, ATTACK, HOLD, DECAY, SUSTAIN, RELEASE
} ENVELOPE_STATE_T;

// all of these are in seconds except for sustain, which is %-based from 0.0 - 1.0
typedef struct adsr {
  double attack; 
  double hold;
  double decay;
  double sustain;
  double release;

  // bottom ones are practical variables for implementation:
  uint64_t frame_count;
  double release_level;
  ENVELOPE_STATE_T state;
} adsr_t;

/**
 * Returns a number between 0 and 1
 * Increments the frame count. 
 * Changes envelope state as needed.
 *
 * @param adsr adsr envelope
 */
double process_adsr (adsr_t* adsr, int sample_rate);

#endif // ADSR_H
