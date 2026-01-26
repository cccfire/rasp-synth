#ifndef CDSL_EVENT_H
#define CDSL_EVENT_H

#include <stdbool.h>
#include <stdatomic.h>

#include <portaudio.h>
#include <SDL3/SDL_surface.h>

#include "queue.h"

#define EVENT_DATA_SIZE (64) // 64 byte max event data size!
#define QUEUE_SIZE (1024)

typedef struct cdsl_event {
  bool end;
  void (*process_data)(void* app_ctx, void* data);
  uint8_t data[EVENT_DATA_SIZE];
} cdsl_event_t;

DECLARE_QUEUE(cdsl_event_t, cdsl_event, QUEUE_SIZE);
DEFINE_QUEUE(cdsl_event_t, cdsl_event, QUEUE_SIZE, ((cdsl_event_t){.end = true}));


#endif // CDSL_EVENT_H
