#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include "thd_ctrl.h"
#include "request_rx.h"

typedef struct task_param
{
} task_param_t;

static int task_init(task_param_t* param)
{
    return RET_OK;
}

static int task_onceopr(task_param_t* param)
{
    return RET_OK;
}

static int task_clear(task_param_t* param)
{
    return RET_OK;
}

hthd_t request_rx_hdl_new(const char* thdname)
{
    task_param_t param =
    {
    };

    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void request_rx_hdl_release(hthd_t hthd)
{
    return thd_handle_release(hthd);
}
