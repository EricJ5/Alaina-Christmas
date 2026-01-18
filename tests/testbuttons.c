#include "buttons.h"

#include "sighandle.h"
#include "playlist.h"

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
	printf("hey from button loop\n");
	char key = getchar();
	while (running) {
		if (!updated) {
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
				pthread_create(&pid, NULL, update_playlist, NULL);
                        }
		} else if (updated) {
			printf("hey from update");
			mpv_command(mpv, playlistCommand);
			updated = false;
		}
	}
	return NULL;
}

