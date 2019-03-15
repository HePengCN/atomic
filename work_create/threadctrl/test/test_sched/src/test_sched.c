#include <stdbool.h>
#include <pthread.h>
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

#define gettid() syscall(__NR_gettid)


bool g_exit;

void show_current(const char* name)
{
    struct sched_param param;
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &param);
    printf("%s", name);
    switch (policy)
    {
    case SCHED_FIFO:
        printf(", policy = SCHED_FIFO");
        break;
    case SCHED_RR:
        printf(", policy = SCHED_RR");
        break;
    case SCHED_OTHER:
        printf(", policy = SCHED_OTHER");
        break;
    default:
        printf(", policy = UNKNOWN");
        break;
    }

    printf(", param.sched_priority: %d; tid: %lu, gettid: %ld\n", param.sched_priority, pthread_self(), gettid());

}


void* start_routine(void *param)
{
    static int i = 0;
    int ret;

    int size = 1024;
    char* buf = (char*)malloc(size);
    if (buf == NULL)
    {
        return NULL;
    }

    show_current("normal thread");
    while (!g_exit)
    {
        show_current("normal thread");
        printf("normal: %d\n", i++);
        usleep(10);
    }


    //task_clear
    free(buf);

    return param;
}



void* start_routine2(void *param)
{
    static int i = 0;
    int ret;

    int size = 1024;
    char* buf = (char*)malloc(size);
    if (buf == NULL)
    {
        return NULL;
    }

    while (!g_exit)
    {
        show_current("RR thread");
        printf("RR: %d\n", i++);
        usleep(10);
    }


    //task_clear
    free(buf);

    printf("%s exit.\n", __FUNCTION__);
    return param;
}



static int get_thread_policy(pthread_attr_t* attr)
{
    int policy;
    int rs = pthread_attr_getschedpolicy(attr, &policy);
    assert(rs == 0);
    switch (policy)
    {
    case SCHED_FIFO:
        printf("policy = SCHED_FIFO\n");
        break;
    case SCHED_RR:
        printf("policy = SCHED_RR\n");
        break;
    case SCHED_OTHER:
        printf("policy = SCHED_OTHER\n");
        break;
    default:
        printf("policy = UNKNOWN\n");
        break;
    }
    return policy;
}



static void show_thread_priority(int policy)
{
    int priority = sched_get_priority_max(policy);
    assert(priority != -1);
    printf("max_priority = %d\n", priority);
    priority = sched_get_priority_min(policy);
    assert(priority != -1);
    printf("min_priority = %d\n", priority);
}



int main(int argc, char* argv[])
{
    printf("Test start: pid: %d.\n", getpid());

    int ret;

    pthread_attr_t attr;
    struct sched_param sched;
    ret = pthread_attr_init(&attr);
    int policy = get_thread_policy(&attr);
    show_thread_priority(policy);

    g_exit = false;

    pthread_t thread, thread2;

#if 1
    ret = pthread_create(&thread, &attr, start_routine, NULL);
    if (ret != 0)
    {
        printf("can't create thread: %s\n", strerror(errno));
        return -1;
    }
#endif
    struct sched_param param;
    param.sched_priority = 50;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setschedparam(&attr, &param);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret != 0)
    {
        printf("pthread_attr_setinheritsched fail: %s, ret: %d\n", strerror(errno), ret);
        return -1;
    }


    policy = get_thread_policy(&attr);
    show_thread_priority(policy);


    ret = pthread_create(&thread2, &attr, start_routine2, NULL);
    if (ret != 0)
    {
        printf("can't create thread2: %s, ret: %d\n", strerror(errno), ret);
        return -1;
    }


    g_exit = true;

    printf("+sleep+\n");
    sleep(20);
    printf("-sleep-\n");

    ret = pthread_join(thread2, NULL);

    printf("pthread_join(thread2, NULL) ret: %d\n", ret);

    return 0;
}
