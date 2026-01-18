#include "screen.h"
#include "app.h"

void app_init (cdsl_app_t* const app) 
{
  enter_screen(app, app->starting_screen);
  app->init();
}

void app_draw (cdsl_app_t* const app)
{
  app->active_screen->draw(app, app->renderer);
}
