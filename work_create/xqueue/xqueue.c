#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include "xqueue.h"
#include "com_log.h"

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
    int max_count;
    uint32_t msgmaxlen;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} xqueue_t;

static void free_oldest_one(xqueue_t* hqueue)
{
    assert(hqueue->count > 0);
    xnode_t *tmp = hqueue->head;
    hqueue->head = hqueue->head->next;
    free(tmp);
    hqueue->count--;
    if (0 == hqueue->count)
    {
        hqueue->tail = NULL;
    }
}

static void mkdata_for_user(xqueue_t* hqueue, void* pdata, uint32_t* pdata_size)
{
    *pdata_size = hqueue->head->data_size;
    memcpy(pdata, hqueue->head->data, *pdata_size);
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
    xqueue_t* hqueue = (xqueue_t*)malloc(sizeof(xqueue_t));
    if (NULL == hqueue)
    {
        COM_LOG_ERROR("malloc fail: sizeof(xqueue_t): %"PRIu64"\n", (uint64_t)sizeof(xqueue_t));
        return NULL;
    }
    memset(hqueue, 0, sizeof(xqueue_t));
    hqueue->max_count = -1;
    hqueue->msgmaxlen = msgmaxlen;
    pthread_cond_init(&(hqueue->cond), NULL);
    pthread_mutex_init(&(hqueue->mutex), NULL);
    return (hqueue_t)hqueue;
}

/*************************************************
@brief destory the queue
@param hqueue: the handle of the queue.
@return
*************************************************/
void xqueue_destory(hqueue_t queue)
{
    if (NULL == queue)
    {
        return;
    }
    xqueue_t* hqueue = (xqueue_t*)queue;
    while (!xqueue_empty(hqueue))
    {
        free_oldest_one(hqueue);
    }
    pthread_cond_destroy(&(hqueue->cond));
    pthread_mutex_destroy(&(hqueue->mutex));
    free(hqueue);
}

/*************************************************
@brief push one msg to the queue
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int  xqueue_push(hqueue_t queue, const void* pdata, uint32_t data_size)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    if (0 == hqueue->max_count)
    {
        COM_LOG_INFO("max_count has been set to 0, which means diable all push operation.");
        return 0;
    }
    assert(data_size <= hqueue->msgmaxlen);
    int ret = 0;
    pthread_mutex_lock(&(hqueue->mutex));
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

    if (hqueue->max_count > 0 && hqueue->count == hqueue->max_count)
    {
        COM_LOG_INFO("reached max_count: %d, dropped the oldest one.\n", hqueue->max_count);
        free_oldest_one(hqueue);
    }

    if (0 == hqueue->count)
    {
        hqueue->head = new;
    }
    else
    {
        hqueue->tail->next = new;
    }
    hqueue->tail = new;
    hqueue->count++;

end:
    if (0 == ret)
    {
        pthread_cond_signal(&(hqueue->cond));
    }
    pthread_mutex_unlock(&(hqueue->mutex));
    return ret;
}


/*************************************************
@brief check if the queue is empty
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xqueue_empty(hqueue_t queue)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    bool ret = true;
    pthread_mutex_lock(&(hqueue->mutex));
    ret = (0 == hqueue->count);
    pthread_mutex_unlock(&(hqueue->mutex));
    return ret;
}


/*************************************************
@brief try pop the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xqueue_try_pop(hqueue_t queue, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    bool ret = true;
    pthread_mutex_lock(&(hqueue->mutex));
    if (0 == hqueue->count)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(hqueue, pdata, pdata_size);

    free_oldest_one(hqueue);
end:
    pthread_mutex_unlock(&(hqueue->mutex));
    return ret;
}


/*************************************************
@brief wait until the queue is not empty, then pop the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_pop(hqueue_t queue, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    int ret = 0;
    pthread_mutex_lock(&(hqueue->mutex));
    if (0 == hqueue->count)
    {
        ret = pthread_cond_wait(&(hqueue->cond), &(hqueue->mutex));
        if (0 != ret)
        {
            COM_LOG_ERROR("pthread_cond_wait return fail, error no: %d\n", ret);
            goto end;
        }
    }

    mkdata_for_user(hqueue, pdata, pdata_size);

    free_oldest_one(hqueue);

end:
    pthread_mutex_unlock(&(hqueue->mutex));
    return ret;

}


/*************************************************
@brief try read but not pop the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool  xqueue_try_front(hqueue_t queue, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    bool ret = true;
    pthread_mutex_lock(&(hqueue->mutex));
    if (0 == hqueue->count)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(hqueue, pdata, pdata_size);

end:
    pthread_mutex_unlock(&(hqueue->mutex));
    return ret;

}


/*************************************************
@brief wait until the queue is not empty, then read but not pop the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int  xqueue_wait_and_front(hqueue_t queue, void* pdata, uint32_t* pdata_size)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    int ret = 0;
    pthread_mutex_lock(&(hqueue->mutex));
    if (0 == hqueue->count)
    {
        ret = pthread_cond_wait(&(hqueue->cond), &(hqueue->mutex));
        if (0 != ret)
        {
            COM_LOG_ERROR("pthread_cond_wait return fail, error no: %d\n", ret);
            goto end;
        }
    }

    mkdata_for_user(hqueue, pdata, pdata_size);

end:
    pthread_mutex_unlock(&(hqueue->mutex));
    return ret;

}


/*************************************************
@brief get how many data in the queu
@param hqueue: the handle of the queue.
@return data cout
*************************************************/
uint32_t xqueue_size(hqueue_t queue)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    uint32_t size = 0;
    pthread_mutex_lock(&(hqueue->mutex));
    size = hqueue->count;
    pthread_mutex_unlock(&(hqueue->mutex));
    return size;
}


/*************************************************
@brief check the max length of the queue support
@param hqueue: the handle of the queue.
@return max length
*************************************************/
uint32_t xqueue_msg_maxlen(hqueue_t queue)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    return hqueue->msgmaxlen;
}

/*************************************************
@brief set the max item count of the queue.
       when the item number reachs the max count, the oldest item will be dropped.
@param hqueue: the handle of the queue.
       count: <0 means no limit; = 0 diable all push operation; >0  means count is the max number.
@return void
*************************************************/
void xqueue_set_max_count(hqueue_t queue, int max_count)
{
    assert(NULL != queue);
    xqueue_t* hqueue = (xqueue_t*)queue;
    pthread_mutex_lock(&(hqueue->mutex));
    hqueue->max_count = max_count;
    pthread_mutex_unlock(&(hqueue->mutex));
}

