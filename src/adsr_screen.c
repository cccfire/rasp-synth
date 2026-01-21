#include "adsr_screen.h"

#include <stdbool.h>

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "screen.h"

// thickens line horizontally and vertically 
static void __render_thick_line (SDL_Renderer* renderer, float x1, float y1
  , float x2, float y2, int thickness)
{
  for (int i = 0; i < thickness; i++) {
    SDL_RenderLine(renderer, x1 + i, y1, x2 + i, y2);
    SDL_RenderLine(renderer, x1, y1 + i, x2, y2 + i);
  }
}

static void __render_axis (SDL_Renderer* renderer, const float margin, const int width_split, const int thickness)
{
  // get the size of the render output
  int width, height;
  SDL_GetRenderOutputSize(renderer, &width, &height);

  const float second_width = (float) width * (1.0 - margin * 2) / width_split;

  // just draw axes?
  __render_thick_line(renderer, 
    (float) width * margin, 
    (float) height * (1.0 - margin), 
    (float) width * (1.0 - margin),
    (float) height * (1.0 - margin),
    thickness
  );

  __render_thick_line(renderer, 
    (float) width * margin, 
    (float) height * (1.0 - margin), 
    (float) width * margin,
    (float) height * margin,
    thickness
  );

  float tickmark_size = height * margin / 2;
  for (int i = 0; i <= width_split; i++) {
    __render_thick_line(renderer,
      i * second_width + width * margin,
      (float) height * (1.0 - margin) - tickmark_size/2, 
      i * second_width + width * margin,
      (float) height * (1.0 - margin) + tickmark_size/2,
      thickness
    );
  }
}

static void __render_background_lines (SDL_Renderer* renderer, const float margin, const int width_split)
{
  // get the size of the render output
  int width, height;
  SDL_GetRenderOutputSize(renderer, &width, &height);

  const float second_width = (float) width * (1.0 - margin * 2) / width_split;

  float tickmark_size = height * margin / 2;
  for (int i = 0; i <= width_split; i++) {
    SDL_RenderLine(renderer,
      i * second_width + width * margin,
      (float) height * margin,
      i * second_width + width * margin,
      (float) height * (1.0 - margin) - tickmark_size/2 
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
  out_screen->event_callback = (void (*) (cdsl_app_t*, SDL_Event*, void*)) adsr_event_callback;
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

  // line thickness
  const int thickness = 5;

  // we want one second to be approximately 1/4 of the screen
  int width_split = 4;
  float second_width = (float) width * (1.0 - margin * 2) / width_split;

  // draw indicators for seconds
  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
  __render_background_lines(renderer, margin, width_split);

  // draw line for attack
  SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255); // red (maroon)
                                                    
  float prev_x = ((float) width) * margin;
  float prev_y = ((float) height) * (1.0 - margin);

  float next_x = ((float) width) * margin + second_width * ctx->attack;
  float next_y = (float) height * margin;

  __render_thick_line(renderer, prev_x, prev_y, next_x, next_y, thickness);

  // draw line for hold 
  SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // green (darker green)

  prev_x = next_x;
  prev_y = next_y;

  next_x += second_width * ctx->hold;

  __render_thick_line(renderer, prev_x, prev_y, next_x, next_y, thickness);

  // draw line for decay/sustain
  SDL_SetRenderDrawColor(renderer, 218, 165, 32, 255); //  yellow (goldenrod)

  prev_x = next_x;

  next_x += second_width * ctx->decay;
  next_y += (float) height * (1.0 - 2 * margin) * (1 - ctx->sustain);
                                                       
  __render_thick_line(renderer, prev_x, prev_y, next_x, next_y, thickness);

  // draw line for release
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); //  blue (blue)
                                                    
  prev_x = next_x;
  prev_y = next_y;

  next_x += second_width * ctx->release;
  next_y = (float) height * (1.0 - margin);

  __render_thick_line(renderer, prev_x, prev_y, next_x, next_y, thickness);

  SDL_SetRenderDrawColor(renderer, 55, 55, 55, 255);

  __render_rect_frame(renderer, frame_margin);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  __render_axis(renderer, margin, width_split, 2);
}

void adsr_event_callback (cdsl_app_t* app, SDL_Event* event, adsr_ctx_t* ctx)
{
}



