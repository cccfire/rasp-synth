#ifndef CDSL_ADSR_SCREEN_H
#define CDSL_ADSR_SCREEN_H

#include <float.h>

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_events.h>

#include "screen.h"
#include "adsr.h"
#include "app.h"

/**
 * Creates an adsr screen and context.
 *
 * @param[out] out_screen  output screen
 * @param[out] out_ctx     output context 
 */
void create_adsr_screen (cdsl_screen_t* const out_screen, adsr_t* const out_ctx);

void adsr_init (cdsl_app_t* app, adsr_t* ctx);
void adsr_on_enter (cdsl_app_t* app, adsr_t* ctx);
void adsr_on_exit (cdsl_app_t* app, adsr_t* ctx);
void adsr_draw (cdsl_app_t* app, SDL_Renderer* renderer, adsr_t* ctx);
void adsr_event_callback (cdsl_app_t* app, const SDL_Event* event, adsr_t* ctx);

#endif // CDSL_ADSR_SCREEN_H

