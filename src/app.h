#ifndef CDSL_APP_H
#define CDSL_APP_H

#include <stdbool.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

// forward decls
typedef struct screen cdsl_screen_t;

typedef struct app {
  SDL_Renderer* const renderer;
  cdsl_screen_t* const starting_screen;
  cdsl_screen_t* active_screen;

  void (*init) (void*);
  void (*on_draw) (void*);
  int (*audiogen_callback) ( const void*, void*, unsigned long, 
      const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);
  void (*event_callback) (SDL_Event* event);
} cdsl_app_t;

void app_init (cdsl_app_t* const app, void* app_ctx);
void app_draw (cdsl_app_t* const app, void* app_ctx);

#endif // CDSL_APP_H
