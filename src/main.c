#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include <portaudio.h>
#include <portmidi.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "empty_stubs.h"

#include "adsr_screen.h"
#include "raspsynth.h"
#include "screen.h"
#include "app.h"

#define SAMPLE_RATE (44100)

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

void __pmerror_check(PaError pmerr)
{
  if (pmerr != pmNoError)
    printf(  "PortMidi error: %s\n", Pm_GetErrorText( pmerr ) );
  assert(pmerr == pmNoError);
}

int32_t __time_proc(void *time_info) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;  // milliseconds
}

int main(int argc, char *argv[]) {
  // Initialization
  SDL_Init(SDL_INIT_VIDEO);

  PaError paerr = Pa_Initialize();
  __paerror_check(paerr);

  PmError pmerr = Pm_Initialize();
  __pmerror_check(pmerr);
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

  /**
   * using portmidi to open a MIDI input stream for development purposes
   */

  int num_devices = Pm_CountDevices();
  const PmDeviceInfo* info = Pm_GetDeviceInfo(num_devices - 1);
  assert(info != NULL);
  printf( "Opening Input Stream from Device %d: %s\n", num_devices - 1, info->name );

  PortMidiStream* pm_stream;
  PmEvent pm_buffer;
  void* time_info;

  pmerr = Pm_OpenInput(
    &pm_stream,
    num_devices - 1,
    NULL,
    512,
    __time_proc,
    time_info);
  __pmerror_check(pmerr);

  assert (pm_stream != NULL);

  cdsl_screen_t adsr_screen;
  adsr_ctx_t adsr_ctx;
  create_adsr_screen(&adsr_screen, &adsr_ctx);

  cdsl_app_t app = {
    .renderer = renderer,
    .starting_screen = &adsr_screen,
    .init = &empty_init,
    .on_draw = &empty_init
  };

  raspsynth_ctx_t raspsynth_ctx;

  // populates "app" with function pointers
  create_raspsynth(&app, &raspsynth_ctx);

  app_init(&app, &raspsynth_ctx);

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

    // Poll for Portmidi events
    if (Pm_Read(pm_stream, &pm_buffer, 1)) {
      PmMessage message = pm_buffer.message;
      printf("Midi message: %d\n", Pm_MessageData1(message));
    }

    app_draw(&app, &raspsynth_ctx);

    // TODO: Should it be the screen's responsibility to present?
    SDL_RenderPresent(renderer);
    
    /*
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
    */
  }

  // End program, clean up loose ends.
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  paerr = Pa_StopStream(stream);
  __paerror_check(paerr);
  paerr = Pa_CloseStream(stream);
  __paerror_check(paerr);

  paerr = Pa_Terminate();
  __paerror_check(paerr);

  pmerr = Pm_Close(pm_stream);
  __pmerror_check(pmerr);

  pmerr = Pm_Terminate();
  __pmerror_check(pmerr);

  destroy_raspsynth(&app, &raspsynth_ctx);

  return 0;
}
