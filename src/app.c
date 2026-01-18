#include "screen.h"
#include "app.h"

void app_init (cdsl_app_t* const app, void* ctx) 
{
  enter_screen(app, app->starting_screen);
  app->init(ctx);
}

void app_draw (cdsl_app_t* const app, void* ctx)
{
  app->active_screen->draw(app, app->renderer, ctx);
}
