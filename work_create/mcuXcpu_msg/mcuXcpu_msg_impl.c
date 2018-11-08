/*
*******************************************************************
 * Copyright (C):
 * FileName: mcuXcpu_msg.h
 * Author:        Version :          Date:
 * Description:
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
      David    96/10/12     1.0     build this moudle
*******************************************************************
*/
#include <stdlib.h>
#include <string.h>
#include "mcuXcpu_msg.h"
#include "mcuXcpu_msg_impl.h"

void* mcuXcpu_msg_new(uint32_t len)
{
    mcuXcpu_msg_t *pMsg = (mcuXcpu_msg_t*)malloc(sizeof(mcuXcpu_msg_t) + len);;
    if(NULL == pMsg) {
        return NULL;
    }
    pMsg->len = len;
    memset(pMsg->data, 0, len);
    return pMsg;
}

int mcuXcpu_msg_destory(void* pMsg)
{
    if(NULL != pMsg) {
        free(pMsg);
    }
    pMsg = NULL;
    return 0;
}

uint32_t mcuXcpu_msg_size(const void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return ((mcuXcpu_msg_t*)pMsg)->len;
}

char* mcuXcpu_msg_data(void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return ((mcuXcpu_msg_t*)pMsg)->data;
}

const char* mcuXcpu_msg_data_const(const void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return (const char*)(((mcuXcpu_msg_t*)pMsg)->data);
}


mcuXcpu_msg_type_e mcuXcpu_msg_type(const void* pMsg)
{
    if(NULL == pMsg) {
        return XMSG_TYPE_ERROR;
    }
    if(0 == mcuXcpu_msg_size(pMsg)) {
        return XMSG_TYPE_EMPTY;
    }

    return XMSG_TYPE_UNKNOWN;
}


/*WILL NOT SUPPORT: see header comments*/
/*
int mcuXcpu_msg_copy(void* dest, void* src)
{
}
*/



