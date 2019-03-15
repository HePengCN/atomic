/*************************************************
@file cpu_rd.c
@brief the implementation of affairs in thread of read cpu msg center
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
#include "cont.h"
#include "xqueue.h"
#include "mxc_msg.h"
#include "thd_ctl.h"
#include "cpu_rd.h"
#include "com_log.h"

#define TIMEOUT (3000)//ms

typedef struct task_param
{
    void *task_private;
    handle_t hmsgc;
    destaddr_map_t hmap;
    hqueue_t hqueue;
} task_param_t;

static int task_init(task_param_t* param)
{
    param->task_private = cont_new(MCU_MSG_MAXLEN);
    if (NULL == param->task_private)
    {
        COM_LOG_ERROR("cont_new ret fail\n");
        return -1;
    }
    return 0;
}

static int task_onceopr(task_param_t* param)
{
    static unsigned char cmsg_buf[MAX_MSG_DATA_SIZE];
    int ret = 0;
    destaddr_map_t map = param->hmap;
    handle_t msgc = param->hmsgc;
    hqueue_t queue = param->hqueue;
    hcont_t cnter = param->task_private;
    mxc_msg_type_e type;

    cont_reset(cnter);
    memset(cmsg_buf, 0, sizeof(cmsg_buf));
    message_t msg;
    msg.data = cmsg_buf;
    ret = msgc_wait_msg(msgc, TIMEOUT);
    if (0 != ret)
    {
        COM_LOG_WARN("msgc_wait_msg timeout\n");
        return 0;
    }

    ret = msgc_recv_msg(msgc, &msg);
    if (0 != ret)
    {
        COM_LOG_ERROR("msgc_recv_msg ret fail\n");
        return -1;
    }

    type = cpu_msg_type(&msg);
    if (MSG_TYPE_EMPTY == type || MSG_TYPE_UNKNOWN == type)
    {
        COM_LOG_WARN("msgc_recv_msg got unuseable type msg: %s\n", msg_type_str(type));
        return 0;
    }

    cpumsg2mcumsg(&msg, cnter);
    ret = xqueue_push(queue, cont_data_const(cnter), cont_used(cnter));
    if (0 != ret)
    {
        COM_LOG_ERROR("xqueue_push ret fail\n");
        return -1;
    }

    if (!is_cpu_ask(type))
    {
        return 0;
    }

    if (0 == strcmp(msg.header.addr.source, get_dft_destaddr(map, type)))
    {
        return 0;
    }

    insert_destaddr(map, type, msg.header.addr.source);
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

hthd_t cpu_rd_hdl_new(const char thdname[16], handle_t hmsgc, destaddr_map_t hmap, hqueue_t hqueue)
{
    assert(NULL != hmsgc);
    assert(NULL != hmap);
    assert(NULL != hqueue);
    task_param_t param =
    {
        .task_private = NULL, // will managed by task
        .hmsgc = hmsgc,
        .hmap = hmap,
        .hqueue = hqueue
    };
    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void cpu_rd_hdl_release(hthd_t hthd)
{
    return thd_handle_release(hthd);
}
