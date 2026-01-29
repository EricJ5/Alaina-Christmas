#ifndef MPVUTILS_HEADER
#define MPVUTILS_HEADER


#include <mpv/client.h>

void set_speed(mpv_handle *handle, double speedFloat, char* speed);

double get_playback_left(mpv_handle *handle);

void seek_to_time(mpv_handle *handle, double seektime);

void stop(mpv_handle *handle);

void unstop(mpv_handle *handle);

void *mpv_event_loop(void *handle);

#endif //MPVUTILS_HEADER
