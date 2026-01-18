#include "speedloop.h"

#include "mpvutils.h"
#include "sighandle.h"


#include <time.h>



void speed_loop(mpv_handle *handle) {

	struct timespec polltime;
	struct timespec stoptime;
	polltime.tv_sec = 0;
	polltime.tv_nsec = 1000000L;
	stoptime.tv_sec = 0;
	stoptime.tv_nsec = 500000000L;
	double topSpeed = 3.00f;
	double botSpeed = .35;
	double speedFloat = .35;
	double speedIncrement = .001f;
	char speed[15];
	double normalspeed = 1.00f;
	while (running) {
		if (speedFloat == topSpeed) {
			speedIncrement = .001;
		} else if (speedFloat == botSpeed) {
			speedIncrement = -.001;
		}
		speedFloat += speedIncrement;
		set_speed(handle, speedFloat, speed);
		nanosleep(&polltime, NULL);


	}	
}
