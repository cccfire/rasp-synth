#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include <portaudio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "empty_stubs.h"

#include "midi_support.h"
#include "adsr.h"
#include "adsr_screen.h"
#include "raspsynth.h"
#include "screen.h"
#include "app.h"

#define MAX_VOICES (512)


void __print_device_info()
{
  int num_devices = Pm_CountDevices();
  for (int i = 0; i < num_devices; i++) {
    const PmDeviceInfo* info = Pm_GetDeviceInfo(i);
    assert(info != NULL);
    printf( "Device %d: %s\n", i, info->name );
  }
}

void __paerror_check(PaError paerr)
{
  if (paerr != paNoError)
    printf(  "PortAudio error: %s\n", Pa_GetErrorText( paerr ) );
  assert(paerr == paNoError);
}

int main(int argc, char *argv[]) {
  // Initialization
  SDL_Init(SDL_INIT_VIDEO);

  PaError paerr = Pa_Initialize();
  __paerror_check(paerr);

  __print_device_info();

  bool done = false;

  /**
   * SDL init
   */

  // Create window
  SDL_Window *window = SDL_CreateWindow(
    "Raspsynth", // window title
    800, // window width px
    600, // window height px
    //0
    SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALWAYS_ON_TOP // flags - we want the window to always be active
  );

  // Check that window creation was successful
  if (window == NULL) {
    // otherwise error
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

  ////




  raspsynth_ctx_t raspsynth_ctx;

  cdsl_app_t app = {
    .renderer = renderer,
    .init = &empty_init,
    .on_draw = &empty_init
  };


  // populates "app" with function pointers
  create_raspsynth(&app, &raspsynth_ctx, MAX_VOICES);

  cdsl_screen_t adsr_screen;
  create_adsr_screen(&adsr_screen, &(raspsynth_ctx.amp_adsr));

  app.starting_screen = &adsr_screen;

  app_init(&app, &raspsynth_ctx);

  /**
   * using portmidi to open a MIDI input stream for development purposes
   */
  pthread_t midi_thread;
  create_midi_thread(&midi_thread, &app, &raspsynth_ctx, &done);
  

  PaStream* stream;
  paerr = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paFloat32,  /* 32 bit floating point output */
                                SAMPLE_RATE,
                                256,        /* frames per buffer */
                                app.audiogen_callback,
                                (void*) &raspsynth_ctx);

  __paerror_check(paerr);

  paerr = Pa_StartStream(stream);
  __paerror_check(paerr);
  
  while (!done) {
    SDL_Event event;

    app_draw(&app, &raspsynth_ctx);

    // TODO: Should it be the screen's responsibility to present?
    SDL_RenderPresent(renderer);
    
    /*
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
    */

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }

      // keyboard escape
      if (event.type == SDL_EVENT_KEY_DOWN) {
          if (event.key.key == SDLK_ESCAPE) {
              done = true;
          }
      }

      app_event(&app, &event, &raspsynth_ctx);
    }

    int dropped_a = atomic_load_explicit(&raspsynth_ctx.dropped_voices, memory_order_acquire);
    int dropped_b = atomic_load_explicit(&raspsynth_ctx.voice_events.dropped_count, memory_order_acquire);

    if (dropped_a + dropped_b > 0) {
      fprintf(stderr, "dropped %d %d\n", dropped_a, dropped_b);
      fflush(stderr);
    }
  }

  // End program, clean up loose ends.
  pthread_join(midi_thread, NULL);
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  paerr = Pa_StopStream(stream);
  __paerror_check(paerr);
  paerr = Pa_CloseStream(stream);
  __paerror_check(paerr);

  paerr = Pa_Terminate();
  __paerror_check(paerr);


  destroy_raspsynth(&app, &raspsynth_ctx);

  return 0;
}
