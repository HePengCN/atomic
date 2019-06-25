#include <stdio.h>
#include "sigterm_handler.h"

int main (int argc, char* argv[]) {
    printf("Test start: pid: %d.\n", getpid());

    signal_sigterm(NULL);

    wait_sigterm();
    printf("receive SIGTERM. exit.\n");

    return 0;
}
