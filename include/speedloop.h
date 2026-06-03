#ifndef SPEEDLOOP_HEADER
#define SPEEDLOOP_HEADER


#include <mpv/client.h>
#include <stdatomic.h>


void speed_loop(mpv_handle *handle);
extern atomic_int minVoltage;


#endif //SPEEDLOOP_HEADER
