#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include "xstack.h"
#include "com_log.h"

typedef struct xnode
{
    struct xnode *next;
    struct xnode *prev;
    uint32_t data_size;
    char data[0];
} xnode_t;

typedef struct xstack
{
    xnode_t *head;
    xnode_t *tail;
    uint32_t count;
    int max_count;
    uint32_t msgmaxlen;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} xstack_t;

static void free_newest_one(xstack_t* hstack)
{
    assert(hstack->count > 0);
    xnode_t *tmp = hstack->head;
    hstack->head = hstack->head->prev;
    if (NULL != hstack->head)
    {
        hstack->head->next = NULL;
    }
    free(tmp);
    hstack->count--;
    if (0 == hstack->count)
    {
        hstack->tail = NULL;
    }
}

static void free_oldest_one(xstack_t* hstack)
{
    assert(hstack->count > 0);
    xnode_t *tmp = hstack->tail;
    hstack->tail = hstack->tail->next;
    if (NULL != hstack->tail)
    {
        hstack->tail->prev = NULL;
    }
    free(tmp);
    hstack->count--;
    if (0 == hstack->count)
    {
        hstack->head = NULL;
    }
}

static void mkdata_for_user(xstack_t* hstack, void* pdata, uint32_t* pdata_size)
{
    *pdata_size = hstack->head->data_size;
    memcpy(pdata, hstack->head->data, *pdata_size);
}

/*************************************************
@brief create a new thread safe stack, in which random length data can push and pop,
       so you should indicate the max length of data you will deal with.
       should call xstack_destory when user done.
@param msgmaxlen:
       the max length of data you will deal with;
@return if ok: the new handle;
        if fail: return NULL.
*************************************************/
hstack_t xstack_create(uint32_t msgmaxlen)
{
    xstack_t* hstack = (xstack_t*)malloc(sizeof(xstack_t));
    if (NULL == hstack)
    {
        COM_LOG_ERROR("malloc fail: sizeof(xstack_t): %"PRIu64"\n", (uint64_t)sizeof(xstack_t));
        return NULL;
    }
    memset(hstack, 0, sizeof(xstack_t));
    hstack->max_count = -1;
    hstack->msgmaxlen = msgmaxlen;
    pthread_cond_init(&(hstack->cond), NULL);
    pthread_mutex_init(&(hstack->mutex), NULL);
    return (hstack_t)hstack;
}

/*************************************************
@brief destory the stack
@param hstack: the handle of the stack.
@return
*************************************************/
void xstack_destory(hstack_t stack)
{
    if (NULL == stack)
    {
        return;
    }
    xstack_t* hstack = (xstack_t*)stack;
    while (!xstack_empty(hstack))
    {
        free_newest_one(hstack);
    }
    pthread_cond_destroy(&(hstack->cond));
    pthread_mutex_destroy(&(hstack->mutex));
    free(hstack);
}

/*************************************************
@brief push one msg to the stack
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int  xstack_push(hstack_t stack, const void* pdata, uint32_t data_size)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    if (0 == hstack->max_count)
    {
        COM_LOG_INFO("max_count has been set to 0, which means diable all push operation.");
        return 0;
    }
    assert(data_size <= hstack->msgmaxlen);
    int ret = 0;
    pthread_mutex_lock(&(hstack->mutex));
    xnode_t* new = (xnode_t*)malloc(sizeof(xnode_t) + data_size);
    if (NULL == new)
    {
        COM_LOG_ERROR("malloc xnode fail: sizeof(xnode_t): %"PRIu64", data_size: %u\n", (uint64_t)sizeof(xnode_t), data_size);
        ret = -1;
        goto end;
    }
    memset(new, 0, sizeof(xnode_t) + data_size);
    memcpy(new->data, pdata, data_size);
    new->data_size = data_size;

    if (hstack->max_count > 0 && hstack->count == hstack->max_count)
    {
        COM_LOG_INFO("reached max_count: %d, dropped the oldest one.\n", hstack->max_count);
        free_oldest_one(hstack);
    }

    new->prev = hstack->head;
    if (0 == hstack->count)
    {
        hstack->tail = new;
    }
    else
    {
        hstack->head->next = new;
    }
    hstack->head = new;
    hstack->count++;

end:
    if (0 == ret)
    {
        pthread_cond_signal(&(hstack->cond));
    }
    pthread_mutex_unlock(&(hstack->mutex));
    return ret;
}


/*************************************************
@brief check if the stack is empty
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xstack_empty(hstack_t stack)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    bool ret = true;
    pthread_mutex_lock(&(hstack->mutex));
    ret = (0 == hstack->count);
    pthread_mutex_unlock(&(hstack->mutex));
    return ret;
}


/*************************************************
@brief try pop the head data from the stack,
       ok if stack is not empty, faile if the stack is empty.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xstack_try_pop(hstack_t stack, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    bool ret = true;
    pthread_mutex_lock(&(hstack->mutex));
    if (0 == hstack->count)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(hstack, pdata, pdata_size);

    free_newest_one(hstack);
end:
    pthread_mutex_unlock(&(hstack->mutex));
    return ret;
}


/*************************************************
@brief wait until the stack is not empty, then pop the data.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xstack_wait_and_pop(hstack_t stack, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    int ret = 0;
    pthread_mutex_lock(&(hstack->mutex));
    if (0 == hstack->count)
    {
        ret = pthread_cond_wait(&(hstack->cond), &(hstack->mutex));
        if (0 != ret)
        {
            COM_LOG_ERROR("pthread_cond_wait return fail, error no: %d\n", ret);
            goto end;
        }
    }

    mkdata_for_user(hstack, pdata, pdata_size);

    free_newest_one(hstack);

end:
    pthread_mutex_unlock(&(hstack->mutex));
    return ret;

}


/*************************************************
@brief try read but not pop the head data from the stack,
       ok if stack is not empty, faile if the stack is empty.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xstack_try_front(hstack_t stack, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    bool ret = true;
    pthread_mutex_lock(&(hstack->mutex));
    if (0 == hstack->count)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(hstack, pdata, pdata_size);

end:
    pthread_mutex_unlock(&(hstack->mutex));
    return ret;

}


/*************************************************
@brief wait until the stack is not empty, then read but not pop the data.
@param hstack: the handle of the stack.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int  xstack_wait_and_front(hstack_t stack, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    int ret = 0;
    pthread_mutex_lock(&(hstack->mutex));
    if (0 == hstack->count)
    {
        ret = pthread_cond_wait(&(hstack->cond), &(hstack->mutex));
        if (0 != ret)
        {
            COM_LOG_ERROR("pthread_cond_wait return fail, error no: %d\n", ret);
            goto end;
        }
    }

    mkdata_for_user(hstack, pdata, pdata_size);

end:
    pthread_mutex_unlock(&(hstack->mutex));
    return ret;

}


/*************************************************
@brief get how many data in the queu
@param hstack: the handle of the stack.
@return data cout
*************************************************/
uint32_t xstack_size(hstack_t stack)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    uint32_t size = 0;
    pthread_mutex_lock(&(hstack->mutex));
    size = hstack->count;
    pthread_mutex_unlock(&(hstack->mutex));
    return size;
}


/*************************************************
@brief check the max length of the stack support
@param hstack: the handle of the stack.
@return max length
*************************************************/
uint32_t xstack_msg_maxlen(hstack_t stack)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    return hstack->msgmaxlen;
}

/*************************************************
@brief set the max item count of the stack.
       when the item number reachs the max count, the oldest item will be dropped.
@param hstack: the handle of the stack.
       count: <0 means no limit; = 0 diable all push operation; >0  means count is the max number.
@return void
*************************************************/
void xstack_set_max_count(hstack_t stack, int max_count)
{
    assert(NULL != stack);
    xstack_t* hstack = (xstack_t*)stack;
    pthread_mutex_lock(&(hstack->mutex));
    hstack->max_count = max_count;
    pthread_mutex_unlock(&(hstack->mutex));
}

