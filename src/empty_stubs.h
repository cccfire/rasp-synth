#ifndef CDSL_EMPTY_STUBS_H
#define CDSL_EMPTY_STUBS_H

#include <float.h>

#include <portaudio.h>
#include <SDL3/SDL_render.h>

#include "screen.h"
#include "app.h"

void empty_init (void* ptr) 
{
}

void empty_screen (cdsl_app_t* app, void* ptr) 
{
}

void empty_draw (cdsl_app_t* app, SDL_Renderer* renderer, void* ptr) 
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

int empty_audiogen_callback( 
  const void* input,
  void* output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData )
{
  return 0;
}

cdsl_screen_t cdsl_minimal_screen = {
  .is_initialized = false,
  .ctx = NULL,
  .init = &empty_screen,
  .on_enter = &empty_screen,
  .on_exit = &empty_screen,
  .draw = &empty_draw,
};

#endif // CDSL_EMPTY_STUBS_H
