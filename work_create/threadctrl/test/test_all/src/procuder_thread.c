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
#include "xqueue.h"
#include "producer_thread.h"

typedef struct task_param
{
    char* buf;
    int size;
    hqueue_t hqueue;
} task_param_t;


static int task_init(task_param_t* param)
{
    param->size = xqueue_msg_max_len(param->hqueue);
    param->buf = (char*)malloc(param->size);
    if (param->buf == NULL)
    {
        return -1;
    }
    memset(param->buf, 0, param->size);
    strncpy(param->buf, "abcdefghijklmnopqrstuvwxyz", 26);
    param->size = 26;
    return 0;
}

static int task_onceopr(task_param_t* param)
{
    int ret;
    ret = xqueue_enqueue(param->hqueue, param->buf, param->size);
    if (ret != 0)
    {
        printf("xqueue_enqueue fail: %d\n", ret);
        return RET_EXIT;
    }

    param->buf[0] += 1;

    if (param->buf[0] > 'z')
    {
        param->buf[0] = 'a';
    }

    usleep(1000);
    return RET_OK;
}

static int task_clear(task_param_t* param)
{
    if (param->buf != NULL)
    {
        free(param->buf);
    }
    return 0;
}


hthd_t producer_thread_handler_new(const char thdname[16], void* hqueue)
{
    task_param_t param =
    {
        .hqueue = hqueue
    };

    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void producer_thread_handler_release(hthd_t hthd)
{
    return thd_handle_release(hthd);
}
