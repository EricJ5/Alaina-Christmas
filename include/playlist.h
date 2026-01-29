#ifndef PLAYLIST_HEADER
#define PLAYLIST_HEADER

#include <stdatomic.h>
#include <stdbool.h>

extern atomic_flag downloading;
extern char *playlist;
extern const char *playlistCommand[];
extern char **environ;
extern atomic_bool updated;
// this is assigned in musicbox.c
extern char *ytUrl;

void *update_playlist(void *arg);

#endif //PLAYLIST_HEADER
