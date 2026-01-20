#include <stdbool.h>

#include <portaudio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "adsr_screen.h"
#include "screen.h"
#include "app.h"


void empty_init (void* ptr) {
}

void empty_screen (cdsl_app_t* app, void* ptr) {
}

void empty_draw (cdsl_app_t* app, SDL_Renderer* renderer, void* ptr) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

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

  cdsl_screen_t minimal_screen = {
    .is_initialized = false,
    .ctx = NULL,
    .init = &empty_screen,
    .on_enter = &empty_screen,
    .on_exit = &empty_screen,
    .draw = &empty_draw,
  };

  cdsl_screen_t adsr_screen;
  adsr_ctx_t adsr_ctx;
  create_adsr_screen(&adsr_screen, &adsr_ctx);

  cdsl_app_t app = {
    .renderer = renderer,
    .starting_screen = &adsr_screen,
    .init = &empty_init,
    .on_draw = &empty_init
  };

  app_init(&app, NULL);
  
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

    app_draw(&app, app.active_screen->ctx);

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
  return 0;
}
