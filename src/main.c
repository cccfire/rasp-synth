#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  bool done = false;

  SDL_Window *window = SDL_CreateWindow(
    "Test", // window title
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
  
  while (!done) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }
    }
    
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
