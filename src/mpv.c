#include "utils.h"
#include <syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

volatile bool press = false;
volatile int Pevent = 0;
void PressEvent(int event) {
	press = true;
	Pevent = event;
}
void *ButtonLoop(void *arg) {
	for (;;) {
		printf("hey from button loop\n");
		int key = getchar();
		if (key == 's') {
			PressEvent(1);
		}
		else if (key == 'p') {
			PressEvent(-1);
		}
	}
}
int main(int argc,char *argv[]) {
	printf("%s", argv[1]);
	printf("hey-1\n");
	mpv_handle *mpv = mpv_create();
	
	mpv_set_option_string(mpv, "vid", "no");
    	mpv_set_option_string(mpv, "volume", "30");
	mpv_set_property_string(mpv, "ytdl", "yes");
	mpv_initialize(mpv);
   
	pthread_t id;	
	pthread_create(&id, NULL, ButtonLoop, NULL);
	char *playlisturl = argv[1];
	const char *playlistcommand[] ={"loadfile", playlisturl , "replace", NULL};
	const char *nextcommand[] ={"playlist-next", NULL};
	printf("hey0\n");
	const char *prevcommand[] ={"playlist-prev", NULL};
	printf("hey1\n");
	mpv_command(mpv, playlistcommand);
	
		
	
	printf("hey2\n");
	float sfloat = .35f;
	float increment = .001f;
	float max = 3.000f;
	char speed[10];
	for (;;) {
		if (press) {
			if (Pevent == 1) {
				mpv_command(mpv, nextcommand);
			}
			else if (Pevent == -1) {
				mpv_command(mpv, prevcommand);

			}
			press = false;
		}
		if (sfloat >= max) {
		increment = -.001f;
		} else if (sfloat <= .20f) {
		increment = .001f;
		}
		sfloat += increment;
		usleep(10*1000);
		set_speed(mpv, sfloat, speed);
	//	printf("%f\n", sfloat);
	}	
	return 0;
}	

