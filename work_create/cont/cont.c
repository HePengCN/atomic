/*
*******************************************************************
 * Copyright (C):
 * FileName: cont.c
 * Author:        Version :          Date:
 * Description: a memory container for user
 * Version:
 * Function List:
 * History:
      <author>  <time>   <version >   <desc>
*******************************************************************
*/
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include "cont.h"
#include "com_log.h"

#define TAG "CONT"

typedef struct cont
{
    uint32_t size;
    uint32_t used;
    char data[0];
} cont_t;

hcont_t cont_new(uint32_t size)
{
    cont_t *con = (cont_t*)malloc(sizeof(cont_t) + size);
    if (NULL == con)
    {
        COM_LOG_ERROR(TAG, "malloc fail: sizeof(cont_t): %"PRIu64", size: %u\n", (uint64_t)sizeof(cont_t), size);
        return NULL;
    }
    con->size = size;
    con->used = 0;
    memset(con->data, 0, size);
    return con;
}

void cont_destory(hcont_t con)
{
    if (NULL != con)
    {
        free(con);
    }
}

void cont_reset(hcont_t con)
{
    assert(NULL != con);
    cont_t* tmp = (cont_t*)con;
    tmp->used = 0;
    memset(tmp->data, 0, tmp->size);
}

uint32_t cont_size(const hcont_t con)
{
    assert(NULL != con);
    const cont_t* tmp = (const cont_t*)con;
    return tmp->size;
}

uint32_t cont_used(const hcont_t con)
{
    assert(NULL != con);
    const cont_t* tmp = (const cont_t*)con;
    return tmp->used;
}

void cont_set_used(hcont_t con, uint32_t used)
{
    assert(NULL != con);
    cont_t* tmp = (cont_t*)con;
    tmp->used = used;
}

char* cont_data(hcont_t con)
{
    assert(NULL != con);
    cont_t* tmp = (cont_t*)con;
    return tmp->data;
}

const char* cont_data_const(const hcont_t con)
{
    assert(NULL != con);
    const cont_t* tmp = (const cont_t*)con;
    return (const char*)(tmp->data);
}


uint32_t* cont_used_ptr(hcont_t con)
{
    assert(NULL != con);
    cont_t* tmp = (cont_t*)con;
    return &(tmp->used);
}
