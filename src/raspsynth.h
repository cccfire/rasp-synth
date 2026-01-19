#ifndef RASPSYNTH_H
#define RASPSYNTH_H

#include <stdbool.h>

#include <SDL3/SDL_surface.h>

#include "adsr_screen.h"

// forward decls

typedef struct raspsynth {
  adsr_ctx_t* amp_adsr;
  adsr_ctx_t* filter_adsr;
} raspsynth_ctx_t;

#endif // RASPSYNTH_H

