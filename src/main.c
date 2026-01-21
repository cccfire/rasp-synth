#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include <portaudio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "empty_stubs.h"

#include "adsr_screen.h"
#include "raspsynth.h"
#include "screen.h"
#include "app.h"

#define SAMPLE_RATE (44100)


void __error_check(PaError err)
{
  if(err != paNoError)
    printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
  assert(err == paNoError);
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  PaError err = Pa_Initialize();
  __error_check(err);

  bool done = false;

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

  /*
  */

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
  err = Pa_OpenDefaultStream( &stream,
                                0,          /* no input channels */
                                2,          /* stereo output */
                                paFloat32,  /* 32 bit floating point output */
                                SAMPLE_RATE,
                                256,        /* frames per buffer */
                                app.audiogen_callback,
                                (void*) &raspsynth_ctx);

  __error_check(err);

  err = Pa_StartStream(stream);
  __error_check(err);
  
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
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  err = Pa_StopStream(stream);
  __error_check(err);
  err = Pa_CloseStream(stream);
  __error_check(err);

  err = Pa_Terminate();
  __error_check(err);

  return 0;
}
