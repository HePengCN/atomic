#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
/*

In the following shell session, the main thread sends a cancellation request to the other thread:

    $ ./a.out
    New thread started
    cnt = 0
    cnt = 1
    Canceling thread
    Called clean-up handler
    Thread was canceled; cnt = 0

From the above, we see that the thread was canceled, and that the cancellation clean-up handler was called and it reset the value of the
global variable cnt to 0.

In the next run, the main program sets a global variable that causes other thread to terminate normally:

    $ ./a.out x
    New thread started
    cnt = 0
    cnt = 1
    Thread terminated normally; cnt = 2

From  the  above,  we see that the clean-up handler was not executed (because cleanup_pop_arg was 0), and therefore the value of cnt was
not reset.

In the next run, the main program sets a global variable that causes the other thread to terminate  normally,  and  supplies  a  nonzero
value for cleanup_pop_arg:

    $ ./a.out x 1
    New thread started
    cnt = 0
    cnt = 1
    Called clean-up handler
    Thread terminated normally; cnt = 0

In  the  above,  we  see  that  although  the  thread was not canceled, the clean-up handler was executed, because the argument given to
pthread_cleanup_pop() was nonzero.

*/



#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static int done = 0;
static int cleanup_pop_arg = 0;
static int cnt = 0;

static void cleanup_handler(void *arg)
{
    printf("Called clean-up handler\n");
    cnt = 0;
}

static void * thread_start(void *arg)
{
    time_t start, curr;

    printf("New thread started\n");

    pthread_cleanup_push(cleanup_handler, NULL);

    curr = start = time(NULL);

    while (!done)
    {
        pthread_testcancel();           /* A cancellation point */
        if (curr < time(NULL))
        {
            curr = time(NULL);
            printf("cnt = %d\n", cnt);  /* A cancellation point */
            cnt++;
        }
    }
    pthread_cleanup_pop(cleanup_pop_arg);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thr;
    int s;
    void *res;

    s = pthread_create(&thr, NULL, thread_start, NULL);
    if (s != 0)
        handle_error_en(s, "pthread_create");

    sleep(2);           /* Allow new thread to run a while */

    if (argc > 1)
    {
        if (argc > 2)
            cleanup_pop_arg = atoi(argv[2]);
        done = 1;

    }
    else
    {
        printf("Canceling thread\n");
        s = pthread_cancel(thr);
        if (s != 0)
            handle_error_en(s, "pthread_cancel");
    }

    s = pthread_join(thr, &res);
    if (s != 0)
        handle_error_en(s, "pthread_join");

    if (res == PTHREAD_CANCELED)
        printf("Thread was canceled; cnt = %d\n", cnt);
    else
        printf("Thread terminated normally; cnt = %d\n", cnt);
    exit(EXIT_SUCCESS);
}


