/*************************************************
@file cpu_wr.c
@brief the implementation of affairs in thread of write msg center
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
#include "com_log.h"
#include "cpu_wr.h"

typedef struct task_param
{
    void *task_private;// will inited in task_init, used by task_onceopr, released in task_clear
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
    int ret = 0;

    destaddr_map_t map = param->hmap;
    handle_t msgc = param->hmsgc;
    hqueue_t queue = param->hqueue;
    hcont_t cnter = (hcont_t)param->task_private;

    cont_reset(cnter);
    ret = xqueue_wait_and_pop(queue, cont_data(cnter), cont_used_ptr(cnter));
    if (0 != ret)
    {
        COM_LOG_ERROR("xqueue_wait_and_pop ret fail\n");
        return -1;
    }

    mxc_msg_type_e type = mcu_msg_type((const char*)cont_data_const(cnter), cont_used(cnter));
    if (MSG_TYPE_EMPTY == type || MSG_TYPE_UNKNOWN == type)
    {
        COM_LOG_WARN("xqueue_wait_and_pop got unuseable type msg: %s. msg dump: %s\n", msg_type_str(type), mcu_msg_dump_head((const char*)cont_data_const(cnter)));
        return 0;
    }

    message_t msg;
    char dest_addr[MAX_ADDR_LEN];
    int len = mcu_msg_content_len((const char*)cont_data(cnter));
    int msg_id = calc_msg_id_4msgc(type, LOCAL_MODULEID);
    int flag = MSG_TYPE_NORMAL | SEND_TYPE_UCAST;
    int sequence = 0;
    int count = destaddr_count(map, type);

    int i = 0;
    for (i = 0; i < count; i++)
    {
        memset(dest_addr, 0, sizeof(dest_addr));
        get_destaddr(map, type, i, dest_addr);

        assert(strlen(dest_addr) > 0);
        ret = msgc_packet_msg(msgc, &msg, dest_addr, (unsigned char*)cont_data(cnter), len, msg_id, flag, sequence);
        if (0 != ret)
        {
            COM_LOG_ERROR("msgc_packet_msg ret fail\n");
            return -1;
        }
        ret = msgc_send_msg(msgc, &msg);
        if (0 != ret)
        {
            COM_LOG_ERROR("msgc_send_msg ret fail\n");
            return -1;
        }

        if (!is_cpu_ask(type))
        {
            continue;
        }

        if (0 == i)
        {
            continue;
        }

        erase_destaddr(map, type, i);
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

hthd_t cpu_wr_hdl_new(const char thdname[16], handle_t hmsgc, destaddr_map_t hmap, hqueue_t hqueue)
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

void cpu_wr_hdl_release(hthd_t hthd)
{
    return thd_handle_release(hthd);
}
