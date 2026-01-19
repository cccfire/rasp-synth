#ifndef CDSL_ADSR_SCREEN_H
#define CDSL_ADSR_SCREEN_H

#include <float.h>

#include <SDL3/SDL_render.h>

#include "screen.h"
#include "app.h"

// all of these are in seconds except for sustain, which is %-based from 0.0 - 1.0
typedef struct adsr_ctx {
  double attack; 
  double decay;
  double sustain;
  double release;
} adsr_ctx_t;

/**
 * Creates an adsr screen and context.
 *
 * @param[out] out_screen  output screen
 * @param[out] out_ctx     output context 
 */
void create_adsr_screen (cdsl_screen_t* const out_screen, adsr_ctx_t* const out_ctx);

void adsr_init (cdsl_app_t* app, adsr_ctx_t* ctx);
void adsr_on_enter (cdsl_app_t* app, adsr_ctx_t* ctx);
void adsr_on_exit (cdsl_app_t* app, adsr_ctx_t* ctx);
void adsr_draw (cdsl_app_t* app, SDL_Renderer* renderer, adsr_ctx_t* ctx);

#endif // CDSL_ADSR_SCREEN_H

