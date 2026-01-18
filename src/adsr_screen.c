#include "adsr_screen.h"

#include <stdbool.h>

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
  ctx->attack = 0.0;
  ctx->decay = 0.0;
  ctx->sustain = 0.0;
  ctx->release = 0.0;
}

void adsr_on_enter (cdsl_app_t* app, adsr_ctx_t* ctx) {
}

void adsr_on_exit (cdsl_app_t* app, adsr_ctx_t* ctx) {
}

void adsr_draw (cdsl_app_t* app, SDL_Renderer* renderer, adsr_ctx_t* ctx) {
}

