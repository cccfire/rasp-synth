#ifndef CDSL_ADSR_SCREEN_H
#define CDSL_ADSR_SCREEN_H

#include <stdbool.h>

#include <SDL3/SDL.h>

#include "screen.h"
#include "app.h"

typedef struct adsr_screen {
  bool is_initialized = false;

  void (*init) (cdsl_app_t*);
  void (*on_enter) (cdsl_app_t*);
  void (*on_exit) (cdsl_app_t*);
  void (*draw) (cdsl_app_t*, SDL_Renderer*);
} adsr_screen_t;

#endif // CDSL_ADSR_SCREEN_H

