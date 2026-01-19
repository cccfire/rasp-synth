#include "adsr_screen.h"

#include <stdbool.h>

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "screen.h"

void create_adsr_screen (cdsl_screen_t* const out_screen, adsr_ctx_t* const out_ctx) {
  out_screen->is_initialized = false;
  out_screen->ctx = (void*) out_ctx;
  out_screen->init = (void (*) (cdsl_app_t*, void*)) adsr_init;
  out_screen->on_enter = (void (*) (cdsl_app_t*, void*)) adsr_on_enter;
  out_screen->on_exit = (void (*) (cdsl_app_t*, void*)) adsr_on_exit;
  out_screen->draw = (void (*) (cdsl_app_t*, SDL_Renderer*, void*)) adsr_draw;
}

void adsr_init (cdsl_app_t* app, adsr_ctx_t* ctx) {
  // Default values for adsr:
  ctx->attack = 1.0;
  ctx->decay = 1.0;
  ctx->sustain = 0.7;
  ctx->release = 0.5;
}

void adsr_on_enter (cdsl_app_t* app, adsr_ctx_t* ctx) {
}

void adsr_on_exit (cdsl_app_t* app, adsr_ctx_t* ctx) {
}

void adsr_draw (cdsl_app_t* app, SDL_Renderer* renderer, adsr_ctx_t* ctx) {
  

  // first fill black
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // get the size of the render output
  int width, height;
  SDL_GetRenderOutputSize(renderer, &width, &height);

  // we want one second to be approximately 1/4 of the screen
  float second_width = (float) width * 0.9 / 4;

  // draw line for attack
  SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255); // red (maroon)


  SDL_RenderLine(renderer, 
      (float) (width/20), 
      (float) height * 0.95, 
      ((float) width) * 0.05 + second_width * ctx->attack, 
      (float) height * 0.05 
  );

  // draw line for decay/sustain
  SDL_SetRenderDrawColor(renderer, 255, 206, 27, 255); //  yellow (mustard)
  SDL_RenderLine(renderer, 
      ((float) width) * 0.05 + second_width * ctx->attack, 
      (float) height * 0.05, 
      ((float) width) * 0.05 + second_width * ctx->attack + second_width * ctx->decay,
      (float) height * 0.05 + (float) height * 0.9 * (1 - ctx->sustain)
  );

  // draw line for release
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); //  blue (blue)
  SDL_RenderLine(renderer, 
      ((float) width) * 0.05 + second_width * ctx->attack + second_width * ctx->decay,
      (float) height * 0.05 + (float) height * 0.9 * (1 - ctx->sustain),
      ((float) width) * 0.05 + second_width * ctx->attack + second_width * ctx->decay + second_width * ctx->release,
      (float) height * 0.95 
  );
  
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  // draw rectangle frame w/ 5% margins
  SDL_FRect rect = {
    .x = (float) (width/20),
    .y = (float) (height/20),
    .w = (float) (width * 0.9),
    .h = (float) (height * 0.9)
  };

  SDL_RenderRect(renderer, &rect);
}

