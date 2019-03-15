/*************************************************
@file mcu_wr.c
@brief the implementation of affairs in thread of write mcu channel
*************************************************/
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include "thd_ctl.h"
#include "mcu_wr.h"
#include "mcu_chan_io.h"
#include "xqueue.h"
#include "cont.h"
#include "com_log.h"

typedef struct task_param
{
    void *task_private;  // will inited in task_init, used by task_onceopr, released in task_clear
    hmcuchan_t hMcuChan;
    hqueue_t hqueue;
} task_param_t;

static int task_init(task_param_t* param)
{
    assert(NULL != param->hMcuChan);
    assert(NULL != param->hqueue);
    param->task_private = cont_new(xqueue_msg_maxlen(param->hqueue));
    if (NULL == param->task_private)
    {
        COM_LOG_ERROR("cont_new ret fail\n");
        return -1;
    }
    return 0;
}

static int task_onceopr(task_param_t* param)
{
    int ret = 0;
    assert(NULL != param->hMcuChan);
    assert(NULL != param->hqueue);
    assert(NULL != param->task_private);
    hmcuchan_t mch_chan = param->hMcuChan;
    hqueue_t queue = param->hqueue;
    hcont_t cnter = param->task_private;

    cont_reset(cnter);

    ret = xqueue_wait_and_pop(queue, cont_data(cnter), cont_used_ptr(cnter));
    if (0 != ret)
    {
        COM_LOG_ERROR("xqueue_wait_and_pop ret fail, error code: <%d>\n", ret);
        return -1;
    }

    ret = mcu_chan_write_timewait(mch_chan, -1);
    if (0 != ret)
    {
        COM_LOG_ERROR("mcu_chan_write_timewait ret fail, error code: <%d>\n", ret);
        return -1;
    }

    ret = mcu_chan_write(mch_chan, cont_data(cnter), cont_used(cnter));
    if (ret < 0)
    {
        COM_LOG_ERROR("mcu_chan_write ret<%d> fail, error msg: %s\n", ret, strerror(errno));
        return -1;
    }

    return 0;
}

static int task_clear(task_param_t* param)
{
    if (NULL != param->task_private)
    {
        cont_destory(param->task_private);
        param->task_private = NULL;
    }
    return 0;
}

hthd_t mcu_wr_hdl_new(const char thdname[16], void* hMcuChan, void* hqueue)
{
    assert(NULL != hqueue);
    task_param_t param =
    {
        .task_private = NULL,
        .hMcuChan = hMcuChan,
        .hqueue = hqueue
    };

    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void mcu_wr_hdl_release(hthd_t hthd_base)
{
    return thd_handle_release(hthd_base);
}

