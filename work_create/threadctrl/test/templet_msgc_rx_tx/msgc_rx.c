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
#include "msgc.h"
#include "imsgc.h"
#include "cont.h"

#define TIMEOUT (-1)

typedef struct task_param
{
    handle_t hmsgc;
    hqueue_t hqueue;
    struct queue_elem queue_elem;
    hcont_t hcont;
} task_param_t;


/* Handle various types of messages. */
static int convert_to_queue_elem(message_t* msg, struct queue_elem* queue_elem)
{
    //Implements here.
    return 0;
}

static int task_init(task_param_t* param)
{
    param->hmsgc = msgc_register(MSGC_ADDR_CARDATA_PROC_RX);
    if (param->hmsgc == NULL)
    {
        COM_LOG_ERROR("msgc_register fail.\n");
        return RET_EXIT;
    }

    param->hcont = cont_create(MAX_MSG_DATA_SIZE);
    if (param->hcont == NULL)
    {
        COM_LOG_ERROR("cont_create ret fail\n");
        return RET_EXIT;
    }

    return RET_OK;
}

static int task_onceopr(task_param_t* param)
{
    int ret;
    message_t msg;
    msg.data = cont_data(param->hcont);
    ret = msgc_wait_msg(param->hmsgc, TIMEOUT);
    if (ret != 0)
    {
        COM_LOG_WARN("msgc_wait_msg timeout\n");
        return RET_AGAIN;
    }

    ret = msgc_recv_msg(param->hmsgc, &msg);
    if (ret != 0)
    {
        COM_LOG_ERROR("msgc_recv_msg ret fail\n");
        return RET_EXIT;
    }

    ret = convert_to_queue_elem(&msg, &(param->queue_elem));
    if (ret != 0)
    {
        COM_LOG_ERROR("make_can_frame ret fail\n");
        return RET_EXIT;
    }

    ret = xqueue_enqueue(param->hqueue, &(param->queue_elem), sizeof(param->queue_elem));
    if (ret != 0)
    {
        COM_LOG_ERROR("xqueue_enqueue ret fail\n");
        return RET_EXIT;
    }

    return RET_OK;
}

static int task_clear(task_param_t* param)
{
    if (param->hmsgc != NULL)
    {
        msgc_unregister(param->hmsgc);
        param->hmsgc = NULL;
    }

    if (param->hcont != NULL)
    {
        cont_destory(param->hcont);
        param->hcont = NULL;
    }

    return RET_OK;
}

hthd_t msgc_rx_hdl_new(const char thdname[16], hqueue_t hqueue)
{
    assert(hqueue != NULL);

    task_param_t param =
    {
        .hqueue = hqueue
    };

    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void msgc_rx_hdl_release(hthd_t hthd_base)
{
    return thd_handle_release(hthd_base);
}
