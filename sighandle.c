#include "sighandle.h"


volatile sig_atomic_t running = 1;
void handle_sigterm(int sig)
{
    (void)sig;
    running = 0;
}

