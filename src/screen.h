#ifndef CDSL_SCREEN_H
#define CDSL_SCREEN_H

#include <stdbool.h>

#include <SDL3/SDL.h>

#include "app.h"

typedef struct screen {
  bool is_initialized;

  void (*init) (cdsl_app_t*);
  void (*on_enter) (cdsl_app_t*);
  void (*on_exit) (cdsl_app_t*);
  void (*draw) (cdsl_app_t*, SDL_Renderer*);
} cdsl_screen_t;

/**
 * Intended to be the main way to switch screens
 *
 * - exits current active screen
 * - sets active screen to new screen
 * - initializes new screen if new screen has not been initialized
 * - enters new screen
 */
void enter_screen (cdsl_app_t* const app, cdsl_screen_t* const screen);

#endif // CDSL_SCREEN_H
