#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <portmidi.h>

#include "app.h"
#include "midi_support.h"

void __pmerror_check(PmError pmerr)
{
  if (pmerr != pmNoError)
    printf(  "PortMidi error: %s\n", Pm_GetErrorText( pmerr ) );
  assert(pmerr == pmNoError);
}

int32_t __time_proc(void *time_info) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;  // milliseconds
}

typedef struct package {
  PortMidiStream* pm_stream;
  cdsl_app_t* app;
  void* ctx;
  bool* done;
} __package_t;


void* midi_thread_func(__package_t* package) 
{
  PmEvent pm_buffer;
  while (!*package->done) {
    // Poll for Portmidi events
    if (Pm_Read(package->pm_stream, &pm_buffer, 1)) {
      PmMessage message = pm_buffer.message;
      int status = Pm_MessageStatus(pm_buffer.message);
      int command = status & 0xF0;  // Upper 4 bits
      int channel = status & 0x0F;  // Lower 4 bits

      // Note on
      if (command == 0x90) {
        package->app->note_on(
            (int32_t) Pm_MessageData1(pm_buffer.message),
            (int32_t) Pm_MessageData2(pm_buffer.message),
            package->ctx);
      }
      // Note off
      else if (command == 0x80) {
        package->app->note_off(
            (int32_t) Pm_MessageData1(pm_buffer.message),
            package->ctx);
      }
    }
  }

  PmError pmerr = Pm_Close(package->pm_stream);
  __pmerror_check(pmerr);

  pmerr = Pm_Terminate();
  __pmerror_check(pmerr);

  return NULL;
}

void create_midi_thread(pthread_t* out_thread, cdsl_app_t* app, void* ctx, bool* done)
{
  PmError pmerr = Pm_Initialize();
  __pmerror_check(pmerr);
  int num_devices = Pm_CountDevices();
  const PmDeviceInfo* info = Pm_GetDeviceInfo(num_devices - 1);
  assert(info != NULL);
  printf( "Opening Input Stream from Device %d: %s\n", num_devices - 1, info->name );

  PortMidiStream* pm_stream;
  void* time_info;

  pmerr = Pm_OpenInput(
      &pm_stream,
      num_devices - 1,
      NULL,
      512,
      __time_proc,
      time_info);
  __pmerror_check(pmerr);

  assert (pm_stream != NULL);

  __package_t* package = (__package_t*) malloc(sizeof(__package_t));
  assert (package != NULL);
  package->pm_stream = pm_stream;
  package->app = app;
  package->ctx = ctx;
  package->done = done;

  pthread_create(out_thread, NULL, (void* (*) (void*))midi_thread_func, package);
}
