#include "utils.h"
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>
#include <stdbool.h>


#ifndef RESET
#define RESET
volatile bool reset = false;
void StartReset() {
	reset = true;
}
#endif //RESET
int main() {
	for (;;) {
		reset = false;
		mpv_handle *mpv = mpv_create();
		mpv_handle *rmpv = mpv_create();
		mpv_set_option_string(rmpv, "vid", "no");
		mpv_set_option_string(rmpv, "volume", "30");
	
		mpv_set_option_string(mpv, "vid", "no");
		mpv_set_option_string(mpv, "volume", "30");
		mpv_initialize(mpv);
		mpv_initialize(rmpv);


		char *fileurl = "songs/song.wav";
		char *rfileurl = "songs/rsong.wav";
		const char *rcommand[] ={"loadfile", rfileurl, NULL};
		const char *command[] ={"loadfile", fileurl, NULL}; 


		mpv_command(mpv, command);


		mpv_command(rmpv, rcommand);
		stop(rmpv);
	
	
		float sfloat = .35f;
		float increment = .001f;
		bool backwards = false;
		float max = 3.000f;
		char speed[10];
		for (;;) {
			if (reset) {
				break;
			}
    			if (sfloat >= max) {
			increment = -.001f;
			} else if (sfloat <= -3.000f) {
			increment = .001f;
			}
			//printf("%f\n", sfloat);
			usleep(10*1000);
			sfloat += increment;
			if (sfloat < -.35f) {
				if (backwards == false) {
					printf("%s", "heyb\n");
					set_speed(mpv, 1.00f, speed);
					double seek_time = get_playback_left(mpv);
					printf("%f mpv\n", seek_time);
					seek_to_time(rmpv, seek_time);
					double test = get_playback_left(rmpv);
					printf("%f rmpv \n", test);
					stop(mpv);
					unstop(rmpv);
					backwards = true;
				}
				printf("%f\n", sfloat);
				set_speed(rmpv, sfloat, speed);

			} else if (sfloat > .35f) {
				if (backwards == true) {
					printf("%s", "heyf\n");
					set_speed(rmpv, 1.00f, speed);
					double seek_time = get_playback_left(rmpv);
					printf("%f rmpv \n",seek_time);
					seek_to_time(mpv, seek_time);
					double test = get_playback_left(mpv);
					printf("%f mpv \n", test);
					stop(rmpv);
					unstop(mpv);
					backwards = false;
				}
				printf("%f\n", sfloat);
				set_speed(mpv, sfloat, speed);
	
			} else {
			stop(rmpv);
			stop(mpv);
			}
		}
	}
	return 0;
}	

