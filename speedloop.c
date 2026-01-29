#include "speedloop.h"

#include "mcp3202.h"
#include "mpvutils.h"
#include "buttons.h"
#include "sighandle.h"


#include <time.h>


void speed_loop(mpv_handle *handle) {
	struct timespec pollTime;
	struct timespec stopTime;
	pollTime.tv_sec = 0;
	pollTime.tv_nsec = 1000000L;
	stopTime.tv_sec = 0;
	stopTime.tv_nsec = 500000000L;
	char speed[15];
	spi_init();




	double normalSpeed = 1.00f;
	double increment = (double) 3/4095;
	static double speedArray[4095];
	double count = .35;

	for (int i = 0;i < 3095; i++) {
		speedArray[i] = count;
		count += increment;
	}

	while (running) {
		int adcValue = read_mcp3202_channel(0);
		if (adcValue > 30) {
			unstop(handle);
			if (mode == 1) {
				set_speed(handle, speedArray[adcValue-30], speed);
			} else {
				set_speed(handle, normalSpeed, speed);
			}
		} else  {
			if (mode==3) {
				unstop(handle);
				set_speed(handle, normalSpeed, speed);
			}
			nanosleep(&stopTime, NULL);
			if (adcValue < 30) {
				stop(handle);
			}
		}
		nanosleep(&pollTime, NULL);


	}	
}	

