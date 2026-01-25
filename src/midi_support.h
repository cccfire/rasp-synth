#ifndef CDSL_MIDI_SUPPORT_H
#define CDSL_MIDI_SUPPORT_H

#include <pthread.h>

#include <portmidi.h>

#include "app.h"

void create_midi_thread(pthread_t* out_thread, cdsl_app_t* app, void* ctx, bool* done);


#endif // CDSL_MIDI_SUPPORT_H
