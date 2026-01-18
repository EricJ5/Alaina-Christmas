#include "sighandle.h"
#include "mpvutils.h"
#include "playlist.h"
#include "buttons.h"
#include "speedloop.h"


#include <fcntl.h>
#include <spawn.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pthread.h>

char *ytUrl;
int main(int argc,char *argv[]) {

    	struct sigaction sa = {0};
    	sa.sa_handler = handle_sigterm;
    	sigemptyset(&sa.sa_mask);
    	sa.sa_flags = 0;
	printf("hello");
    	sigaction(SIGTERM, &sa, NULL);
    	sigaction(SIGINT,  &sa, NULL);
	ytUrl = argv[1];

	printf("%s", argv[1]);
	mpv_handle *mpv = mpv_create();
	
	mpv_set_option_string(mpv, "softvol", "yes");
	mpv_set_option_string(mpv, "ao", "alsa");
	mpv_set_option_string(mpv, "loop-playlist", "inf");
	mpv_set_option_string(mpv, "vid", "no");
	mpv_initialize(mpv);
	mode_t perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	mkdir("Songs", perms);
	pthread_t mpvThread;
	pthread_create(&mpvThread, NULL, mpv_event_loop, mpv);


	pthread_t buttonThread;
	pthread_create(&buttonThread, NULL, button_loop, mpv);


	

	mpv_command(mpv, playlistCommand);

	pthread_t updateThread;
	pthread_create(&updateThread, NULL, update_playlist, NULL);



	speed_loop(mpv);


	return 0;
}	

