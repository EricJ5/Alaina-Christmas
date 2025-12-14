#include "utils.h"
#include <stdio.h>

void set_speed(mpv_handle *handle, float speednum, char *speed) {
	float speedf = speednum;
	if (speedf < 0) {
	speednum = speednum*-1;
	}
	sprintf(speed, "%f", speedf);
	mpv_set_option_string(handle, "speed", speed);
}
double get_playback_left(mpv_handle *handle) {
	double time_remaining;
	mpv_get_property(handle, "time-remaining", MPV_FORMAT_DOUBLE, &time_remaining); 
	// printf("%f\n", time_remaining);
	return time_remaining;
}
void seek_to_time(mpv_handle *handle, double seektime) {
	char time[20];
	sprintf(time, "%f", seektime);
	mpv_set_property_string(handle, "time-pos", time);
}
void stop(mpv_handle *handle) {
	mpv_set_property_string(handle, "pause", "yes");
}
void unstop(mpv_handle *handle) {
	mpv_set_property_string(handle, "pause", "no");
}
