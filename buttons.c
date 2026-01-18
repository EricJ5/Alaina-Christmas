#include "buttons.h"

#include "playlist.h"
#include "gpio.h"
#include "sighandle.h"

#include <mpv/client.h>
#include <stdio.h>
#include <pthread.h>

int mode = 1;


void *button_loop(void *handle) {
	mpv_handle *mpv = (mpv_handle*)handle;
	const char *nextCommand[] ={"playlist-next", NULL};
	const char *prevCommand[] ={"playlist-prev", NULL};
	const char *volUpCommand[] ={"add", "volume", "5", NULL};
	const char *volDownCommand[] ={"add", "volume", "-5", NULL};
	pthread_t pid;
	gpioinit();
	while (running) {
		if (!updated) {
			if (digitalRead(VOLUME_UP) ==  LOW) {
				printf("volume up\n");
				mpv_command(mpv, volUpCommand);
				delay(200);
			}
                        else if (digitalRead(VOLUME_DOWN) ==  LOW) {
				printf("volume down\n");
				mpv_command(mpv, volDownCommand);
				delay(200);
                        }
                        else if (digitalRead(SKIP) ==  LOW) {
				printf("skip\n");
				mpv_command(mpv, nextCommand);
				delay(200);
                        }
                        else if (digitalRead(PREV) ==  LOW) {
				printf("prev\n");
				mpv_command(mpv, prevCommand);
				delay(200);
                        }
                        else if (digitalRead(MODE) ==  LOW) {
				printf("mode switch \n");
				if (mode == 1) {mode=2;}
				else if (mode == 2) {mode=3;}
				else if (mode == 3) {mode=1;}
				delay(200);
                        }
                        else if (digitalRead(RESET) ==  LOW) {
				printf("reset\n");
				pthread_create(&pid, NULL, update_playlist, NULL);
				delay(200);
                        }
		}
		else {
			printf("hey from update");
			mpv_command(mpv, playlistCommand);
			updated = false;
		}
	}
	return NULL;
}
