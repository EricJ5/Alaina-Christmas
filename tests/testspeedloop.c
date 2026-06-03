#include "speedloop.h"

#include "sighandle.h"


#include <stdatomic.h>
#include <stdio.h>


_Atomic int minVoltage = 30;

void speed_loop(mpv_handle *handle) {
	double normalSpeed = 1.00f;
	double increment = (double) 3/4095;
	static double speedArray[4095];
	double count = .35;

	for (int i = 0;i < 4095; i++) {
		speedArray[i] = count;
		count += increment;
	}
	while (running) {
	}
}
