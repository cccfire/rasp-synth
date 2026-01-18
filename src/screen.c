#include "screen.h"

#include "app.h"

void enter_screen (cdsl_app_t* const app, cdsl_screen_t* const screen)
{
  if (app->active_screen && app->active_screen != screen) {
    app->active_screen->on_exit(app);
  }

  if (!screen->is_initialized) {
    screen->is_initialized = true;
    screen->init(app);
  }

  app->active_screen = screen;
  screen->on_enter(app);
}
