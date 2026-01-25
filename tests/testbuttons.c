#include "buttons.h"

#include "sighandle.h"
#include "playlist.h"

#include <mpv/client.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>


int mode = 1;
char key;
volatile bool keyPress;
void *key_thread(void *arg) {
	while (running) {
		key = getchar();
		keyPress = true;
	}
	return NULL;
}

void *button_loop(void *handle) {
	mpv_handle *mpv = (mpv_handle*)handle;
	const char *nextCommand[] ={"playlist-next", NULL};
	const char *prevCommand[] ={"playlist-prev", NULL};
	const char *volUpCommand[] ={"add", "volume", "5", NULL};
	const char *volDownCommand[] ={"add", "volume", "-5", NULL};
	pthread_t playlistPid;
	pthread_t keyPid;
	pthread_create(&keyPid, NULL, key_thread, NULL);
	printf("hey from button loop\n");
	while (running) {
		if (!updated) {
			if (keyPress) {
				if (key == 'u') {
					printf("volume up\n");
					mpv_command(mpv, volUpCommand);
				} else if (key == 'd') {
					printf("volume down\n");
					mpv_command(mpv, volDownCommand);
				} else if (key ==  's') {
					printf("skip\n");
					mpv_command(mpv, nextCommand);
				} else if (key == 'p') {
					printf("prev\n");
					mpv_command(mpv, prevCommand);
				} else if (key == 'r') {
					printf("reset\n");
					pthread_create(&playlistPid, NULL, update_playlist, NULL);
				}
				keyPress = false;
			}
		} else if (updated) {
			printf("hey from update\n");
			mpv_command(mpv, playlistCommand);
			updated = false;
		}
	}
	return NULL;
}

