#ifndef CDSL_APP_H
#define CDSL_APP_H

#include <stdbool.h>

#include <portmidi.h>
#include <portaudio.h>

#include <SDL3/SDL_render.h>
// forward decls
typedef struct screen cdsl_screen_t;

typedef struct app {
  SDL_Renderer* const renderer;
  cdsl_screen_t* starting_screen;
  cdsl_screen_t* active_screen;

  void (*init) (void* ctx);
  void (*on_draw) (void* ctx);
  int (*audiogen_callback) ( const void*, void*, unsigned long, 
      const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);
  void (*event_callback) (const SDL_Event*, void* ctx);
  void (*note_on) (const int32_t pitch, const int32_t velocity, void* ctx);
  void (*note_off) (const int32_t pitch, void* ctx);
  
  // it's here for the future but unused for raspsynth
  void (*midi_callback) (PmEvent* buffer);
} cdsl_app_t;

void app_init (cdsl_app_t* const app, void* app_ctx);
void app_draw (cdsl_app_t* const app, void* app_ctx);
void app_event (cdsl_app_t* const app, const SDL_Event* event, void* app_ctx);

#endif // CDSL_APP_H
