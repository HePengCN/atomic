#include <sys/syscall.h>
#include <unistd.h>
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
#include <pthread.h>
#include "xqueue.h"
#include "consumer_thread.h"

#define gettid() syscall(__NR_gettid)

typedef struct task_param
{
    char* buf;
    int size;
    hqueue_t hqueue;
} task_param_t;


static int task_init(task_param_t* param)
{
    param->size = xqueue_msg_max_len(param->hqueue);
    printf("comsumer: buf size: %d\n", param->size);
    param->buf = (char*)malloc(param->size);
    if (param->buf == NULL)
    {
        return RET_EXIT;
    }

    memset(param->buf, 0, param->size);
    return RET_OK;
}

static int task_onceopr(task_param_t* param)
{
    int ret = 0;
    ret = xqueue_wait_and_dequeue(param->hqueue, param->buf, &param->size);
    //usleep(10000);
    if (ret != 0)
    {
        printf("xqueue_wait_and_dequeue fail\n");
        return RET_EXIT;
    }


    //pthread_testcancel();
    //if(param->buf != NULL) {
    //printf("param->buf: %p\n", param->buf);
    //pthread_testcancel();
    fprintf(stderr, "[%d]: consumer get: %s. xqueue_size: %u\n", (int)gettid(), param->buf, xqueue_size(param->hqueue));

    //fprintf(stderr, "[%d]\n", (int)gettid());

    //printf("-param->size: %d\n", param->size);
    //param->buf[0] = '1';
    //char a =  param->buf[1];
    //param->buf[1] =  param->buf[0];
    //pthread_testcancel();
    //}
    return RET_OK;
}

static int task_clear(task_param_t* param)
{
    if (param->buf != NULL)
    {
        fprintf(stderr, "comsumer clear done.\n");
        free(param->buf);
        param->buf = NULL;
    }

    return 0;
}


hthd_t consumer_thread_handler_new(const char thdname[16], void* hqueue)
{
    task_param_t param =
    {
        .hqueue = hqueue
    };

    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void consumer_thread_handler_release(hthd_t hthd)
{
    return thd_handle_release(hthd);
}
