#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "ptimer.h"
#include "com_log.h"

#define gettid() syscall(__NR_gettid)

typedef struct ptimer
{
    timer_t timerid;
    sigset_t set;
    siginfo_t info;
} ptimer_t;


int ptimer_sigmask_in_main()
{
    int ret;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);

    ret = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (ret != 0)
    {
        COM_LOG_ERROR("pthread_sigmask fail\n");
        return -1;
    }
    return 0;
}

/*************************************************
@brief create a thread timer,which means SIGALRM will send to the thread indicated by thread_id;
@param thread_id: Linux lWP id.
       if thead_id set to 0, it means create a timer for the caller thread.
@return handle of timer. NULL will be return if fail.
*************************************************/
htimer_t ptimer_create(pid_t thread_id)
{
    int ret;
    ptimer_t *htimer = (ptimer_t *)malloc(sizeof(ptimer_t));
    if (htimer == NULL)
    {
        COM_LOG_ERROR("malloc fail: sizeof(xtimer_t): %"PRIu64"\n", (uint64_t)sizeof(ptimer_t));
        return NULL;
    }
    memset(htimer, 0, sizeof(ptimer_t));

    pid_t tid = thread_id;

    if (tid == 0)
    {
        tid = gettid();
    }

    struct sigevent sev =
    {
        .sigev_notify = SIGEV_THREAD_ID,
        .sigev_signo = SIGALRM,
        ._sigev_un._tid = tid
    };

    //printf("set tid: %ld\n", (long)sev._sigev_un._tid);

    ret = timer_create(CLOCK_REALTIME, &sev, &(htimer->timerid));

    if (ret != 0)
    {
        COM_LOG_ERROR("timer_create fail, ret: %d, %s\n", ret, strerror(errno));
        free(htimer);
        return NULL;
    }

    sigemptyset(&(htimer->set));
    sigaddset(&(htimer->set), SIGALRM);
    return htimer;
}


/*************************************************
@brief
@param start_offset_us:
           If specifies a nonzero value then arms (starts) the timer, setting it to initially expire at the given time. (If
       the timer was already armed, then the previous settings are overwritten.)
           If specifies a zero value then the  timer  is disarmed.

       interval_us:
           Specifies the period of the timer.
           If specifies a zero value, then the timer expires just once, at the time specified by start_offset_ms.
@return 0 if ok, others if fail
*************************************************/
int ptimer_set(htimer_t htimer, uint64_t start_offset_us, uint64_t interval_us)
{
    assert(htimer != NULL);
    ptimer_t *timer = (ptimer_t *)htimer;

    int ret;
    struct itimerspec new_timerspec =
    {
        .it_value = {.tv_sec = start_offset_us / (uint64_t)(1000 * 1000), .tv_nsec = 1000l * (start_offset_us % (uint64_t)(1000 * 1000))},
        .it_interval = {.tv_sec = interval_us / (uint64_t)(1000 * 1000), .tv_nsec = 1000l * (interval_us % (uint64_t)(1000 * 1000))}
    };

    ret = timer_settime(timer->timerid, 0, &new_timerspec, NULL);

    if (ret != 0)
    {
        COM_LOG_ERROR("timer_settime fail: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}


/*************************************************
@brief
@param
@return 0 if ok, others if fail
*************************************************/
int ptimer_wait(htimer_t htimer)
{
    assert(htimer != NULL);
    ptimer_t *timer = (ptimer_t *)htimer;

    int sig;
    siginfo_t info;
    sig = sigwaitinfo(&(timer->set), &info);
    if (sig == -1)
    {
        COM_LOG_ERROR("sigwaitinfo fail: %s\n", strerror(errno));
        return -1;
    }

    //COM_LOG_INFO("tid: %ld, signo: %d, SIGALRM: %d, timer_id: 0x%x\n", gettid(), info.si_signo, SIGALRM, info.si_timerid);

    return 0;
}


/*************************************************
@brief
@param
@return
*************************************************/
void ptimer_destroy(htimer_t htimer)
{
    if (htimer == NULL)
    {
        return;
    }

    ptimer_t *timer = (ptimer_t *)htimer;

    timer_delete(timer->timerid);

    sigemptyset(&(timer->set));

    free(timer);
    return;
}


/*************************************************
@brief calling this function in your main thread to mask SIGALRM in your process.
@param
@return 0 if ok, others if fail
*************************************************/
inline int mask_sigalrm()
{
    int ret;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    ret = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (ret != 0)
    {
        printf("pthread_sigmask fail\n");
        return -1;
    }

    return 0;
}


