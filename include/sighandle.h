#ifndef SIGHANDLE_HEADER
#define SIGHANDLE_HEADER

#include <signal.h>

extern volatile sig_atomic_t running;

void handle_sigterm(int sig);

#endif //SIGHANDLE_HEADER
