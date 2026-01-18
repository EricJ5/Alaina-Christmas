#include "speedloop.h"

#include "mcp3202.h"
#include "mpvutils.h"
#include "buttons.h"
#include "sighandle.h"


#include <time.h>


void speed_loop(mpv_handle *handle) {
	struct timespec polltime;
	struct timespec stoptime;
	polltime.tv_sec = 0;
	polltime.tv_nsec = 1000000L;
	stoptime.tv_sec = 0;
	stoptime.tv_nsec = 500000000L;
	char speed[15];
	spi_init();




	double normalspeed = 1.00f;
	double increment = (double) 3/4095;
	static double speedArray[4095];
	double count = .35;

	for (int i = 0;i < 3095; i++) {
		speedArray[i] = count;
		count += increment;
	}

	while (running) {
		int adc_value = read_mcp3202_channel(0);
		if (adc_value > 30) {
			unstop(handle);
			if (mode == 1) {
				set_speed(handle, speedArray[adc_value-30], speed);
			} else {
				set_speed(handle, normalspeed, speed);
			}
		} else  {
			if (mode==3) {
				unstop(handle);
				set_speed(handle, normalspeed, speed);
			}
			nanosleep(&stoptime, NULL);
			if (adc_value < 30) {
				stop(handle);
			}
		}
		nanosleep(&polltime, NULL);


	}	
}	

