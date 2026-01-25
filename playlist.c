#include "playlist.h"



#include "utils.h"

#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> 
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
	char *tmpPlaylist = "./Songs/tmpplaylist.m3u";

   	char *argv2[] = {
        	"yt-dlp",
        	"--flat-playlist",
        	"--print", "%(id)s.opus",
        	(char *)ytUrl,
        	NULL
    	};
	char *argv1[] = {
        	"yt-dlp",
		"-o",
		"Songs/%(id)s.%(ext)s",
		"--format",
		"bestvideo[format_note!*=Premium]+bestaudio",
        	"-x",
        	"--audio-format", "opus",
        	"--download-archive", (char *)archive,
        	(char *)ytUrl,
       	 	NULL
   	};
	posix_spawnp(&downloadPid, "yt-dlp", NULL, NULL, argv1, environ);
	waitpid(downloadPid, &downloadStatus, 0);
	printf("test print\n");
	if (WIFEXITED(downloadStatus)) {
		int status = WEXITSTATUS(downloadStatus);
		if (status != 0 && status != 2 && status != 101) {
			printf("%d \n", status);
			printf("error when dowloading playlist, check internet connection\n");
		
		} else {printf("playlist downloaded\n");}
	}
	

	unlink(tmpPlaylist);
        posix_spawn_file_actions_t fa;
        posix_spawn_file_actions_init(&fa); 
	int fd = open(tmpPlaylist, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        posix_spawn_file_actions_adddup2(&fa, fd, STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fa, fd);
	posix_spawnp(&playlistWritePid, "yt-dlp", &fa, NULL, argv2, environ);
	waitpid(playlistWritePid, &playlistWriteStatus, 0);


	close(fd);
	posix_spawn_file_actions_destroy(&fa);
	if (WIFEXITED(playlistWriteStatus)) {
		if (WEXITSTATUS(playlistWriteStatus) != 0) {
			printf("error when updating playlist file\n");
    			atomic_flag_clear(&downloading);
			return NULL;
		} else {printf("playlist updated\n");}
	}
	if (compare_two_files(playlist, tmpPlaylist)) {
		unlink(tmpPlaylist);
		printf("no playlist update not updating playlist file\n");
    		atomic_flag_clear(&downloading);
		return NULL;
	}

	unlink(playlist);
	copy_two_files(tmpPlaylist, playlist);
	unlink(tmpPlaylist);
	printf("got here\n");

    	atomic_flag_clear(&downloading);
	updated = true;
	return NULL;	
}











