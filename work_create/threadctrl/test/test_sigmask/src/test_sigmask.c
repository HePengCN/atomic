#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

/* Simple error handling functions */

/*
EXAMPLE
The  program  below  blocks some signals in the main thread, and then creates a dedicated thread to fetch
those signals via sigwait(3).  The following shell session demonstrates its use:

   $ ./a.out &
   [1] 5423
   $ kill -QUIT %1
   Signal handling thread got signal 3
   $ kill -USR1 %1
   Signal handling thread got signal 10
   $ kill -TERM %1
   [1]+  Terminated              ./a.out
*/




#define gettid() syscall(__NR_gettid)

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static void* sig_thread(void *arg)
{
    sigset_t *set = arg;
    int s, sig;

    for (;;) {
        s = sigwait(set, &sig);
        if (s != 0)
            handle_error_en(s, "sigwait");
        printf("Signal handling thread got signal %d\n", sig);
    }
}

int main(int argc, char *argv[])
{
    pthread_t thread;
    sigset_t set;
    int s;

    /* Block SIGQUIT and SIGUSR1; other threads created by main()
       will inherit a copy of the signal mask. */

    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    s = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (s != 0)
        handle_error_en(s, "pthread_sigmask");

    s = pthread_create(&thread, NULL, &sig_thread, (void *) &set);
    if (s != 0)
        handle_error_en(s, "pthread_create");

    /* Main thread carries on to create other threads and/or do
       other work */

    pause();            /* Dummy pause so we can test program */
}

/*

NAME
       pause - wait for signal

SYNOPSIS
       #include <unistd.h>

       int pause(void);

DESCRIPTION
       pause()  causes  the calling process (or thread) to sleep until a signal is
       delivered that either terminates the process or causes the invocation of  a
       signal-catching function.

*/
