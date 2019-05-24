#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/syscall.h>
#include "thd_timer_handle0.h"
#include "thd_ctrl_log.h"

#define gettid() syscall(__NR_gettid)


typedef struct task_param
{
    sigset_t set;
    siginfo_t info;

} task_param_t;

static int task_init(task_param_t* param)
{
    sigemptyset(&(param->set));
    sigaddset(&(param->set), SIGALRM);
    sigaddset(&(param->set), SIGUSR1);
    COM_LOG_INFO("thread %ld init done\n", gettid());
    return RET_OK;
}

static int task_onceopr(task_param_t* param)
{

    int sig;
    sig = sigwaitinfo(&(param->set), &(param->info));
    if (sig == -1)
    {
        COM_LOG_ERROR("sigwaitinfo fail: %s\n", strerror(errno));
        return RET_AGAIN;
    }

    COM_LOG_INFO("tid: %ld, signo: %d, SIGALRM: %d, timer_id: 0x%x\n", gettid(), param->info.si_signo, SIGALRM, param->info.si_timerid);
    return RET_OK;
}

static int task_clear(task_param_t* param)
{
    COM_LOG_INFO("thread %ld exit\n", gettid());

    return RET_OK;
}

hthd_t timer_handle0_new(const char* thdname)
{
    task_param_t param =
    {
    };

    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void timer_handle0_release(hthd_t hthd)
{
    return thd_handle_release(hthd);
}
