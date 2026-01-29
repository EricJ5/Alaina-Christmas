#include "gpio.h"

void gpio_init() {
	wiringPiSetup();
	
	pinMode(VOLUME_UP, INPUT);
	pinMode(VOLUME_DOWN, INPUT);
	pinMode(SKIP, INPUT);
	pinMode(PREV, INPUT);
	pinMode(MODE, INPUT);
	pinMode(RESET, INPUT);

    	pullUpDnControl(VOLUME_UP, PUD_UP);
	pullUpDnControl(VOLUME_DOWN, PUD_UP);
	pullUpDnControl(SKIP, PUD_UP);
	pullUpDnControl(PREV, PUD_UP);
	pullUpDnControl(MODE, PUD_UP);
	pullUpDnControl(RESET, PUD_UP);
}
