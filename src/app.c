#include <assert.h>

#include "screen.h"
#include "app.h"

void app_init (cdsl_app_t* const app, void* ctx) 
{
  enter_screen(app, app->starting_screen);
  app->init(ctx);
}

void app_draw (cdsl_app_t* const app, void* ctx)
{
  app->active_screen->draw(app, app->renderer, app->active_screen->ctx);
}

void app_event (cdsl_app_t* const app, const SDL_Event* event, void* app_ctx)
{
  assert(app->active_screen->is_initialized);
  app->active_screen->event_callback(app, event, app->active_screen->ctx);
  app->event_callback(event, app_ctx);
}
