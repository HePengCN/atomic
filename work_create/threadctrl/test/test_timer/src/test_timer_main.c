#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <pthread.h>
#include "sigterm_handler.h"
#include "thd_ctrl.h"
#include "thd_timer_handle0.h"


#define gettid() syscall(__NR_gettid)

typedef hthd_t (*cfp_t)(const char*, ...);

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void sighandler(int sig)
{
    printf("tid: %ld, sig: %d\n", gettid(), sig);
}

void input_command()
{
    char buf[128];
    memset(buf, 0, sizeof(128));

    int i = 0;
    printf("INPUT COMMAND or CTRL-D:\n");
    while (NULL != fgets(buf, sizeof(buf), stdin))
    {
        int len = strlen(buf);
        if (len > 0)
        {
            buf[len - 1] = '\0'; //  '\n' to '\0'
        }
        printf("GET COMMAND: %s\n", buf);
        (void)system(buf);
        printf("INPUT COMMAND or CTRL-D:\n");

        memset(buf, 0, sizeof(128));
    }
}


int main(int argc, char* argv[])
{
    //pthread_mutex_lock(&mutex);

    int ret;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);
    ret = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (ret != 0)
    {
        printf("pthread_sigmask fail\n");
        return -1;
    }


    cfp_t func;

    func = (cfp_t)timer_handle0_new;

    hthd_t timer_handle0, timer_handle1, timer_handle2, timer_handle3;

    timer_handle0 = timer_handle0_new("timer_handle0");

    timer_handle1 = timer_handle0_new("timer_handle1");

    timer_handle2 = func("timer_handle2");

    timer_handle3 = timer_handle1_new("timer_handle3");

    ret = thd_create_and_start(timer_handle0);
    if (ret != 0)
    {
        printf("thd_create(timer_handle0) fail, ret: %d\n", ret);
        return -1;
    }

    ret = thd_create_and_start(timer_handle1);
    if (ret != 0)
    {
        printf("thd_create(timer_handle1) fail, ret: %d\n", ret);
        return -1;
    }

    ret = thd_create_and_start(timer_handle2);
    if (ret != 0)
    {
        printf("thd_create(timer_handle2) fail, ret: %d\n", ret);
        return -1;
    }

    ret = thd_create_and_start(timer_handle3);
    if (ret != 0)
    {
        printf("thd_create(timer_handle2) fail, ret: %d\n", ret);
        return -1;
    }


    timer_t timerid;
    struct sigevent sev =
    {
        .sigev_notify = SIGEV_THREAD_ID,
        .sigev_signo = SIGALRM,
        ._sigev_un._tid = thd_get_tid(timer_handle0)
    };

    printf("set tid: %ld\n", (long)sev._sigev_un._tid);


    ret = timer_create(CLOCK_REALTIME, &sev, &timerid);

    if (ret != 0)
    {
        printf("timer_create fail, ret: %d, %s\n", ret, strerror(errno));
        return -1;
    }

    //printf("create timerid: 0x%x\n", timerid);

    struct itimerspec new_timerspec =
    {
        .it_value = {.tv_sec = 1, .tv_nsec = 0},
        .it_interval = {.tv_sec = 1, .tv_nsec = 0}
    };

    ret = timer_settime(timerid, 0, &new_timerspec, NULL);



    timer_t timerid1;
    struct sigevent sev1 =
    {
        .sigev_notify = SIGEV_THREAD_ID,
        .sigev_signo = SIGALRM,
        ._sigev_un._tid = thd_get_tid(timer_handle1)
    };

    printf("set tid: %ld\n", (long)sev1._sigev_un._tid);

    ret = timer_create(CLOCK_REALTIME, &sev1, &timerid1);

    if (ret != 0)
    {
        printf("timer_create fail, ret: %d, %s\n", ret, strerror(errno));
        return -1;
    }

    //printf("create timerid: 0x%x\n", timerid);

    struct itimerspec new_timerspec1 =
    {
        .it_value = {.tv_sec = 1, .tv_nsec = 0},
        .it_interval = {.tv_sec = 1, .tv_nsec = 0}
    };

    ret = timer_settime(timerid1, 0, &new_timerspec1, NULL);




    timer_t timerid2;
    struct sigevent sev2 =
    {
        .sigev_notify = SIGEV_THREAD_ID,
        .sigev_signo = SIGUSR1,
        ._sigev_un._tid = thd_get_tid(timer_handle2)
    };

    printf("set tid: %ld\n", (long)sev2._sigev_un._tid);

    ret = timer_create(CLOCK_REALTIME, &sev2, &timerid2);

    if (ret != 0)
    {
        printf("timer_create fail, ret: %d, %s\n", ret, strerror(errno));
        return -1;
    }

    //printf("create timerid: 0x%x\n", timerid);

    struct itimerspec new_timerspec2 =
    {
        .it_value = {.tv_sec = 2, .tv_nsec = 0},
        .it_interval = {.tv_sec = 2, .tv_nsec = 0}
    };

    ret = timer_settime(timerid2, 0, &new_timerspec2, NULL);

#if 0
    signal_sigterm(NULL);
    wait_sigterm();
#else
    //pause();
    input_command();
#endif

    thd_check_alive_and_destroy(timer_handle0);

    thd_check_alive_and_destroy(timer_handle1);

    thd_check_alive_and_destroy(timer_handle2);

    thd_check_alive_and_destroy(timer_handle3);


    timer_handle0_release(timer_handle0);
    timer_handle0_release(timer_handle1);
    timer_handle0_release(timer_handle2);
    timer_handle0_release(timer_handle3);

    return 0;
}
