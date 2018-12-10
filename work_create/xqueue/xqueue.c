#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include "xqueue.h"
#include "com_log.h"

#define TAG "XQUEUE"

typedef struct xnode
{
    struct xnode *next;
    uint32_t data_size;
    char data[0];
} xnode_t;

typedef struct xqueue
{
    xnode_t *head;
    xnode_t *tail;
    uint32_t count;
    uint32_t msgmaxlen;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} xqueue_t;

static void mkdata_for_user(xqueue_t* hQueue, void* pData, uint32_t* pData_size)
{
    *pData_size = hQueue->head->data_size;
    memcpy(pData, hQueue->head->data, *pData_size);
}


/*************************************************
@brief create a new thread safe FIFO queue, in which random length data can push and pop,
       so you should indicate the max length of data you will deal with.
       should call xqueue_destory when user done.
@param msgmaxlen:
       the max length of data you will deal with;
@return if ok: the new handle;
        if fail: return NULL.
*************************************************/
hqueue_t xqueue_new(uint32_t msgmaxlen)
{
    xqueue_t* hQueue = (xqueue_t*)malloc(sizeof(xqueue_t));
    if (NULL == hQueue)
    {
        COM_LOG_ERROR(TAG, "malloc fail: sizeof(xqueue_t): %"PRIu64"\n", (uint64_t)sizeof(xqueue_t));
        return NULL;
    }
    memset(hQueue, 0, sizeof(xqueue_t));
    hQueue->msgmaxlen = msgmaxlen;
    pthread_cond_init(&(hQueue->cond), NULL);
    pthread_mutex_init(&(hQueue->mutex), NULL);
    return (hqueue_t)hQueue;
}

/*************************************************
@brief destory the queue
@param hQueue: the handle of the queue.
@return
*************************************************/
void xqueue_destory(hqueue_t queue)
{
    if (NULL == queue)
    {
        return;
    }
    xqueue_t* hQueue = (xqueue_t*)queue;
    xnode_t *tmp = NULL;
    while (!xqueue_empty(hQueue))
    {
        tmp = hQueue->head->next;
        free(hQueue->head);
        hQueue->head = tmp;
        hQueue->count--;
    }
    pthread_cond_destroy(&(hQueue->cond));
    pthread_mutex_destroy(&(hQueue->mutex));
    free(hQueue);
}

/*************************************************
@brief push one msg to the queue
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int  xqueue_push(hqueue_t queue, const void* pData, uint32_t data_size)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    assert(data_size <= hQueue->msgmaxlen);
    int ret = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    xnode_t* new = (xnode_t*)malloc(sizeof(xnode_t) + data_size);
    if (NULL == new)
    {
        COM_LOG_ERROR(TAG, "malloc xnode fail: sizeof(xnode_t): %"PRIu64", data_size: %u\n", (uint64_t)sizeof(xnode_t), data_size);
        ret = -1;
        goto end;
    }
    memset(new, 0, sizeof(xnode_t) + data_size);
    memcpy(new->data, pData, data_size);
    new->data_size = data_size;

    if (0 == hQueue->count)
    {
        hQueue->head = new;
    }
    else
    {
        hQueue->tail->next = new;
    }
    hQueue->tail = new;
    hQueue->count++;

end:
    if (0 == ret)
    {
        pthread_cond_signal(&(hQueue->cond));
    }
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;
}


/*************************************************
@brief check if the queue is empty
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xqueue_empty(hqueue_t queue)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    bool ret = true;
    pthread_mutex_lock(&(hQueue->mutex));
    ret = (0 == hQueue->count);
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;
}


/*************************************************
@brief try pop the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xqueue_try_pop(hqueue_t queue, void* pData, uint32_t* pData_size)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    bool ret = true;
    pthread_mutex_lock(&(hQueue->mutex));
    if (0 == hQueue->count)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(hQueue, pData, pData_size);

    xnode_t *tmp = hQueue->head;
    hQueue->head = hQueue->head->next;
    free(tmp);
    hQueue->count--;

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;
}


/*************************************************
@brief wait until the queue is not empty, then pop the data.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_pop(hqueue_t queue, void* pData, uint32_t* pData_size)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    int ret = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    if (0 == hQueue->count)
    {
        ret = pthread_cond_wait(&(hQueue->cond), &(hQueue->mutex));
        if (0 != ret)
        {
            COM_LOG_ERROR(TAG, "pthread_cond_wait return fail, error no: %d\n", ret);
            goto end;
        }
    }

    mkdata_for_user(hQueue, pData, pData_size);

    xnode_t *tmp = hQueue->head;
    hQueue->head = hQueue->head->next;
    free(tmp);
    hQueue->count--;

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;

}


/*************************************************
@brief try read but not pop the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xqueue_try_front(hqueue_t queue, void* pData, uint32_t* pData_size)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    bool ret = true;
    pthread_mutex_lock(&(hQueue->mutex));
    if (0 == hQueue->count)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(hQueue, pData, pData_size);

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;

}


/*************************************************
@brief wait until the queue is not empty, then read but not pop the data.
@param hQueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int  xqueue_wait_and_front(hqueue_t queue, void* pData, uint32_t* pData_size)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    int ret = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    if (0 == hQueue->count)
    {
        ret = pthread_cond_wait(&(hQueue->cond), &(hQueue->mutex));
        if (0 != ret)
        {
            COM_LOG_ERROR(TAG, "pthread_cond_wait return fail, error no: %d\n", ret);
            goto end;
        }
    }

    mkdata_for_user(hQueue, pData, pData_size);

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;

}


/*************************************************
@brief get how many data in the queu
@param hQueue: the handle of the queue.
@return data cout
*************************************************/
uint32_t xqueue_size(hqueue_t queue)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    uint32_t size = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    size = hQueue->count;
    pthread_mutex_unlock(&(hQueue->mutex));
    return size;
}


/*************************************************
@brief check the max length of the queue support
@param hQueue: the handle of the queue.
@return max length
*************************************************/
uint32_t xqueue_msg_maxlen(hqueue_t queue)
{
    assert(NULL != queue);
    xqueue_t* hQueue = (xqueue_t*)queue;
    return hQueue->msgmaxlen;
}
