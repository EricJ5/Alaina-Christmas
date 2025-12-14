
#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <mpv/client.h>
void seek_to_time(mpv_handle *handle, double seektime);
void stop(mpv_handle *handle);
void unstop(mpv_handle *handle);
void set_speed(mpv_handle *handle, float speednum, char *speed);
double get_playback_left(mpv_handle *handle);

#endif // UTILS_HEADER

