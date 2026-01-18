#include "playlist.h"





#include <spawn.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>



char *playlist = "./Songs/playlist.m3u";
const char *playlistCommand[] ={"loadfile", "./Songs/playlist.m3u", "replace", NULL};
atomic_flag downloading = ATOMIC_FLAG_INIT;
atomic_bool updated = false;



void *update_playlist(void *arg) {
	while (atomic_flag_test_and_set(&downloading)) {
	};
	pid_t downloadPid;
	pid_t playlistWritePid;
	int downloadStatus;
	int playlistWriteStatus;
	char *Songs = "Songs";
	char *archive = "./Songs/archive.txt";
   	char *argv2[] = {
        	"yt-dlp",
        	"--flat-playlist",
        	"--print", "%(title)s.mp3",
        	(char *)ytUrl,
        	NULL
    	};
	char *argv1[] = {
        	"yt-dlp",
		"-o",
		"Songs/%(title)s.%(ext)s",
        	"-x",
        	"--audio-format", "mp3",
        	"--download-archive", (char *)archive,
        	(char *)ytUrl,
       	 	NULL
   	};
	posix_spawnp(&downloadPid, "yt-dlp", NULL, NULL, argv1, environ);
	waitpid(downloadPid, &downloadStatus, 0);

	if (WIFEXITED(downloadStatus)) {
		if (WEXITSTATUS(downloadStatus) != 0) {
			printf("error when dowloading playlist, check internet connection");
			return NULL;
		
		} else {printf("playlist downloaded");}
	}




	unlink(playlist);
        posix_spawn_file_actions_t fa;
        posix_spawn_file_actions_init(&fa); 
	int fd = open(playlist, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        posix_spawn_file_actions_adddup2(&fa, fd, STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fa, fd);
	posix_spawnp(&playlistWritePid, "yt-dlp", &fa, NULL, argv2, environ);
	waitpid(playlistWritePid, &playlistWriteStatus, 0);
	posix_spawn_file_actions_destroy(&fa);
	if (WIFEXITED(playlistWriteStatus)) {
		if (WEXITSTATUS(playlistWriteStatus) != 0) {
			printf("error when updating playlist file");
			return NULL;
		
		} else {printf("playlist updated");}
	}
    	atomic_flag_clear(&downloading);
	updated = true;
	return NULL;	
}

