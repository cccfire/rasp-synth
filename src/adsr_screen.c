#include "adsr_screen.h"

#include <stdbool.h>

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "screen.h"

static void __render_axis (SDL_Renderer* renderer, const float margin, const int width_split)
{
  // get the size of the render output
  int width, height;
  SDL_GetRenderOutputSize(renderer, &width, &height);

  const float second_width = (width - margin * 2) / width_split;

  // just draw axes?
  SDL_RenderLine(renderer, 
    (float) width * margin, 
    (float) height * (1.0 - margin), 
    (float) width * (1.0 - margin),
    (float) height * (1.0 - margin)
  );

  SDL_RenderLine(renderer, 
    (float) width * margin, 
    (float) height * (1.0 - margin), 
    (float) width * margin,
    (float) height * margin
  );

  float tickmark_size = height * margin / 2;
  for (int i = 0; i <= width_split; i++) {
    SDL_RenderLine(renderer,
      (width * (1.0 - margin * 2) * i / width_split) + width * margin,
      (float) height * (1.0 - margin) - tickmark_size/2, 
      (width * (1.0 - margin * 2) * i / width_split) + width * margin,
      (float) height * (1.0 - margin) + tickmark_size/2
    );
  }
}

static void __render_rect_frame (SDL_Renderer* renderer, float margin)
{
  // get the size of the render output
  int width, height;
  SDL_GetRenderOutputSize(renderer, &width, &height);

  // draw rectangle frame w/ 5% margins
  SDL_FRect rect = {
    .x = (float) width * margin,
    .y = (float) height * margin,
    .w = (float) width * (1.0 - margin * 2),
    .h = (float) height * (1.0 - margin * 2)
  };

  SDL_RenderRect(renderer, &rect);
}

void create_adsr_screen (cdsl_screen_t* const out_screen, adsr_ctx_t* const out_ctx) 
{
  out_screen->is_initialized = false;
  out_screen->ctx = (void*) out_ctx;
  out_screen->init = (void (*) (cdsl_app_t*, void*)) adsr_init;
  out_screen->on_enter = (void (*) (cdsl_app_t*, void*)) adsr_on_enter;
  out_screen->on_exit = (void (*) (cdsl_app_t*, void*)) adsr_on_exit;
  out_screen->draw = (void (*) (cdsl_app_t*, SDL_Renderer*, void*)) adsr_draw;
}

void adsr_init (cdsl_app_t* app, adsr_ctx_t* ctx) 
{
  // Default values for adsr:
  ctx->attack = 0.3;
  ctx->hold = 0.7;
  ctx->decay = 1.0;
  ctx->sustain = 0.7;
  ctx->release = 0.5;
}

void adsr_on_enter (cdsl_app_t* app, adsr_ctx_t* ctx) 
{
}

void adsr_on_exit (cdsl_app_t* app, adsr_ctx_t* ctx) 
{
}

void adsr_draw (cdsl_app_t* app, SDL_Renderer* renderer, adsr_ctx_t* ctx) 
{
  

  // first fill black
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // get the size of the render output
  int width, height;
  SDL_GetRenderOutputSize(renderer, &width, &height);

  const float margin = 0.05;
  const float frame_margin = 0.03;

  // we want one second to be approximately 1/4 of the screen
  int width_split = 4;
  float second_width = (float) width * (1.0 - margin * 2) / width_split;

  // draw line for attack
  SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255); // red (maroon)
                                                    
  float prev_x = ((float) width) * margin;
  float prev_y = ((float) height) * (1.0 - margin);

  float next_x = ((float) width) * margin + second_width * ctx->attack;
  float next_y = (float) height * margin;

  SDL_RenderLine(renderer, prev_x, prev_y, next_x, next_y);

  // draw line for hold 
  SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // green (darker green)

  prev_x = next_x;
  prev_y = next_y;

  next_x += second_width * ctx->hold;

  SDL_RenderLine(renderer, prev_x, prev_y, next_x, next_y);

  // draw line for decay/sustain
  SDL_SetRenderDrawColor(renderer, 255, 206, 27, 255); //  yellow (mustard)

  prev_x = next_x;

  next_x += second_width * ctx->decay;
  next_y += (float) height * (1.0 - 2 * margin) * (1 - ctx->sustain);
                                                       
  SDL_RenderLine(renderer, prev_x, prev_y, next_x, next_y);

  // draw line for release
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); //  blue (blue)
                                                    
  prev_x = next_x;
  prev_y = next_y;

  next_x += second_width * ctx->release;
  next_y = (float) height * (1.0 - margin);

  SDL_RenderLine(renderer, prev_x, prev_y, next_x, next_y);

  SDL_SetRenderDrawColor(renderer, 55, 55, 55, 255);

  __render_rect_frame(renderer, frame_margin);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  __render_axis(renderer, margin, width_split);
}



