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
#include "msgc.h"
#include "imsgc.h"
#include "thd_ctrl.h"
#include "cont.h"
#include "xqueue.h"


typedef struct task_param
{
    handle_t hmsgc;
    hqueue_t hqueue;
    struct queue_elem queue_elem;  // can be hcont_t, base type ,any thing;
    hcont_t hcont;  //msgc msg data buff. Can be the same memmory if the dest_addr and msg id is unique
} task_param_t;


static const char* pick_msgc_dest_addr(struct queue_elem* queue_elem)
{
    //Implements here.
    return MSGC_ID_CARDATA_PROC_TX_0000;
}

static int pick_msgc_msg_id(struct queue_elem* queue_elem)
{
    //Implements here.
    return MSGC_ID_CARDATA_PROC_TX_0000;
}

int convert_to_msgc_msg(struct queue_elem* queue_elem, hcont_t hmsg_cont)
{
    //Implements here.
    return cont_used(hmsg_cont);
}

static int task_init(task_param_t* param)
{
    param->hmsgc = msgc_register(MSGC_ADDR_CARDATA_PROC_TX);
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

    ret = xqueue_wait_and_dequeue(param->hqueue, &(param->queue_elem), NULL);

    if (ret != 0)
    {
        COM_LOG_ERROR("xqueue_wait_and_dequeue error\n");
        return RET_EXIT;
    }

    cont_reset(param->hcont);
    const char* dest_addr = pick_msgc_dest_addr(&(param->queue_elem));
    int msgid = pick_msgc_msg_id(&(param->queue_elem));
    convert_to_msgc_msg(&(param->queue_elem), param->hcont);

    ret = msgc_packet_msg(param->hmsgc,
                          &msg,
                          dest_addr,
                          (unsigned char*)cont_data(param->hcont),
                          cont_used(param->hcont),
                          msgid,
                          get_msgc_flag(msgid, dest_addr),
                          0);
    if (ret != 0)
    {
        COM_LOG_ERROR("msgc_packet_msg ret fail\n");
        return RET_EXIT;
    }

    ret = msgc_send_msg(param->hmsgc, &msg);
    if (ret != 0)
    {
        COM_LOG_ERROR("msgc_send_msg ret fail\n");
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

hthd_t msgc_tx_hdl_new(const char thdname[16], hqueue_t hqueue)
{
    assert(hqueue != NULL);

    task_param_t param =
    {
        .hqueue = hqueue,
    };

    return thd_handle_new(thdname, (task_func)task_init, (task_func)task_onceopr, (task_func)task_clear, &param, sizeof(param));
}

void msgc_tx_hdl_release(hthd_t hthd)
{
    return thd_handle_release(hthd);
}
