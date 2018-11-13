/*
*******************************************************************
 * Copyright (C):
 * FileName: xmsg.h
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
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "xmsg.h"

typedef struct xmsg {
    uint32_t allocated;
    uint32_t used;
    char data[0];
} xmsg_t;

void* xmsg_new(uint32_t len)
{
    xmsg_t *pMsg = (xmsg_t*)malloc(sizeof(xmsg_t) + len);;
    if(NULL == pMsg) {
        return NULL;
    }
    pMsg->allocated = len;
    pMsg->used = 0;
    memset(pMsg->data, 0, len);
    return pMsg;
}

int xmsg_destory(void* pMsg)
{
    if(NULL != pMsg) {
        free(pMsg);
    }
    pMsg = NULL;
    return 0;
}

int xmsg_reset(void* pMsg) {
    assert(NULL != pMsg);
    xmsg_t* msg = (xmsg_t*)pMsg;
    msg->used = 0;
    memset(msg->data, 0, msg->allocated);
    return 0;
}

uint32_t xmsg_allocated(const void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return ((xmsg_t*)pMsg)->allocated;
}

uint32_t xmsg_used(const void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return ((xmsg_t*)pMsg)->used;
}

void xmsg_set_used(void* pMsg, uint32_t used)
{
    assert(NULL != pMsg);
    ((xmsg_t*)pMsg)->used = used;
}


char* xmsg_data(void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return ((xmsg_t*)pMsg)->data;
}

const char* xmsg_data_const(const void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return (const char*)(((xmsg_t*)pMsg)->data);
}


uint32_t* xmsg_used_ptr(void* pMsg)
{
    if(NULL == pMsg) {
        return 0;
    }
    return &(((xmsg_t*)pMsg)->used);
}




