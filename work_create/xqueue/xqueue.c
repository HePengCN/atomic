#include <sys/syscall.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "xqueue.h"
#include "xqueue_log.h"

#define PRE_ALLOCATION_COUNT (3)

#define gettid() syscall(__NR_gettid)

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
    uint32_t max_count;
    uint32_t msg_max_len;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    uint32_t pool_used_count;
    uint32_t pool_used_start;
    char mem_pool[0];
} xqueue_t;

static inline size_t max_mem_of_node(uint32_t msg_max_len)
{
    return sizeof(xnode_t) + msg_max_len;
}

static inline void init_mem_pool(xqueue_t* queue)
{
    queue->pool_used_count = 0;
    queue->pool_used_start = 0;
}

static inline bool is_mem_pool_available(xqueue_t* queue)
{
    if (queue->pool_used_count == PRE_ALLOCATION_COUNT)
    {
        return false;
    }
    return true;
}

static inline char* get_mem_from_pool(xqueue_t* queue)
{
    assert(queue->pool_used_count < PRE_ALLOCATION_COUNT);
    int idx = (queue->pool_used_start + queue->pool_used_count) % PRE_ALLOCATION_COUNT;
    char* mem = queue->mem_pool + idx * max_mem_of_node(queue->msg_max_len);

    queue->pool_used_count++;

    //COM_LOG_DEBUG("queue->pool_used_start: %u, queue->pool_used_count: %u\n", queue->pool_used_start, queue->pool_used_count);
    return mem;
}

static inline bool is_mem_in_pool(xqueue_t* queue, const char* mem)
{
    if (mem < queue->mem_pool)
    {
        return false;
    }

    if (mem > queue->mem_pool + (PRE_ALLOCATION_COUNT - 1)*max_mem_of_node(queue->msg_max_len))
    {
        return false;
    }

    return true;
}

static inline void back_mem_to_pool(xqueue_t* queue, char* node)
{
    assert(queue->pool_used_count > 0);

    char* target = queue->mem_pool + queue->pool_used_start * max_mem_of_node(queue->msg_max_len);
    //COM_LOG_DEBUG("queue->mem_pool: %p, node: %p, target: %p\n", queue->mem_pool, node, target);
    assert(node == target);

    memset(target, 0, max_mem_of_node(queue->msg_max_len));
    queue->pool_used_count--;
    queue->pool_used_start++;
    if (queue->pool_used_start == PRE_ALLOCATION_COUNT || queue->pool_used_count == 0)
    {
        queue->pool_used_start = 0;
    }
    //COM_LOG_DEBUG("queue->pool_used_start: %u, queue->pool_used_count: %u\n", queue->pool_used_start, queue->pool_used_count);
}

static void free_oldest_one(xqueue_t* queue)
{
    assert(queue->count > 0);
    xnode_t *tmp = queue->head;
    queue->head = queue->head->next;
    if (is_mem_in_pool(queue, (const char*)tmp))
    {
        back_mem_to_pool(queue, (char*)tmp);
    }
    else
    {
        free(tmp);
    }
    queue->count--;
    if (queue->count == 0)
    {
        queue->tail = NULL;
    }
}

static void mkdata_for_user(xqueue_t* queue, void* data, uint32_t* data_size)
{
    if (data_size != NULL)
    {
        *data_size = queue->head->data_size;
    }

    if (data != NULL && queue->head->data_size != 0)
    {
        memcpy(data, queue->head->data, queue->head->data_size);
    }
}

static struct timespec get_outtime(int msec)
{
    struct timeval now;
    struct timespec outtime;

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + msec / 1000;
    outtime.tv_nsec = now.tv_usec * 1000 + (msec % 1000) * 1000000l;
    return outtime;
}

static int timewait_and_mkdata(hqueue_t hqueue, void* data, uint32_t* data_size, int timeout_msec, bool isdequeue)
{
    assert(hqueue != NULL);
    //assert(data != NULL);

    xqueue_t* queue = (xqueue_t*)hqueue;
    int ret = XQUEUE_RET_OK;
    pthread_mutex_lock(&(queue->mutex));
    if (queue->count == 0)
    {
        if (timeout_msec < 0)
        {
            ret = pthread_cond_wait(&(queue->cond), &(queue->mutex));
        }
        else
        {
            struct timespec outtime = get_outtime(timeout_msec);
            ret = pthread_cond_timedwait(&(queue->cond), &(queue->mutex), &outtime);
        }

        if (ret == ETIMEDOUT)
        {
            COM_LOG_INFO("pthread_cond_timedwait TIMEOUT, error no: %d\n", ret);
            ret = XQUEUE_RET_TIMEOUT;
            goto end;
        }

        if (ret != 0)
        {
            COM_LOG_ERROR("pthread_cond_wait return fail, error no: %d\n", ret);
            ret = XQUEUE_RET_ERROR;
            goto end;
        }
    }

    mkdata_for_user(queue, data, data_size);

    if (isdequeue)
    {
        free_oldest_one(queue);
    }

end:
    pthread_mutex_unlock(&(queue->mutex));
    return ret;
}


/*************************************************
@brief create a new thread safe FIFO queue, in which random length data can push and pop,
       so you should indicate the max length of data you will deal with.
       should call xqueue_destory when user done.
@param msg_max_len:
       the max length of data you will deal with;
@return if ok: the new handle;
        if fail: return NULL.
*************************************************/
hqueue_t xqueue_create(uint32_t msg_max_len)
{
    size_t alloc_size = sizeof(xqueue_t) + PRE_ALLOCATION_COUNT * max_mem_of_node(msg_max_len);

    xqueue_t* hqueue = (xqueue_t*)malloc(alloc_size);
    if (hqueue == NULL)
    {
        COM_LOG_ERROR("malloc fail: sizeof(xqueue_t): %"PRIu64"\n", (uint64_t)sizeof(xqueue_t));
        return NULL;
    }
    memset(hqueue, 0, alloc_size);
    hqueue->head = NULL;
    hqueue->tail = NULL;
    hqueue->max_count = 0;
    hqueue->msg_max_len = msg_max_len;
    pthread_cond_init(&(hqueue->cond), NULL);
    pthread_mutex_init(&(hqueue->mutex), NULL);

    init_mem_pool(hqueue);
    return (hqueue_t)hqueue;
}

/*************************************************
@brief destory the queue
@param hqueue: the handle of the queue.
@return
*************************************************/
void xqueue_destory(hqueue_t hqueue)
{
    if (hqueue == NULL)
    {
        return;
    }
    xqueue_t* queue = (xqueue_t*)hqueue;
    while (queue->count != 0)
    {
        free_oldest_one(queue);
    }
    pthread_cond_destroy(&(queue->cond));
    pthread_mutex_destroy(&(queue->mutex));
    free(hqueue);
}

/*************************************************
@brief enqueue one msg to the queue
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_enqueue(hqueue_t hqueue, const void* data, uint32_t data_size)
{
    assert(hqueue != NULL);
    xqueue_t* queue = (xqueue_t*)hqueue;
    assert(data_size <= queue->msg_max_len);

    if (queue->max_count != 0 && queue->count == queue->max_count)
    {
        COM_LOG_INFO("reached max_count: %d, dropped the oldest one.\n", queue->max_count);
        free_oldest_one(queue);
    }

    xnode_t* new_node = NULL;

    if (is_mem_pool_available(queue))
    {
        new_node = (xnode_t*)get_mem_from_pool(queue);
    }
    else
    {
        new_node = (xnode_t*)malloc(sizeof(xnode_t) + data_size);
        if (new_node == NULL)
        {
            COM_LOG_ERROR("malloc xnode fail: sizeof(xnode_t): %"PRIu64", data_size: %u\n", (uint64_t)sizeof(xnode_t), data_size);
            return XQUEUE_RET_ERROR;
        }
    }

    memset(new_node, 0, sizeof(xnode_t) + data_size);
    if (data != NULL)
    {
        memcpy(new_node->data, data, data_size);
    }
    new_node->data_size = data_size;

    pthread_mutex_lock(&(queue->mutex));

    if (queue->count == 0)
    {
        queue->head = new_node;
    }
    else
    {
        queue->tail->next = new_node;
    }
    queue->tail = new_node;

    queue->count++;

    pthread_cond_signal(&(queue->cond));

    pthread_mutex_unlock(&(queue->mutex));
    return XQUEUE_RET_OK;
}


/*************************************************
@brief check if the queue is empty
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_isempty(hqueue_t hqueue)
{
    assert(hqueue != NULL);
    xqueue_t* queue = (xqueue_t*)hqueue;
    bool ret = true;
    pthread_mutex_lock(&(queue->mutex));
    ret = (queue->count == 0);
    pthread_mutex_unlock(&(queue->mutex));
    return ret;
}


/*************************************************
@brief try dequeue the head data from the queue,
       ok if queue is not empty, fail if the queue is empty.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_try_dequeue(hqueue_t hqueue, void* data, uint32_t* data_size)
{
    assert(hqueue != NULL);
    xqueue_t* queue = (xqueue_t*)hqueue;
    bool ret = true;
    pthread_mutex_lock(&(queue->mutex));
    if (queue->count == 0)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(queue, data, data_size);

    free_oldest_one(queue);
end:
    pthread_mutex_unlock(&(queue->mutex));
    return ret;
}


/*************************************************
@brief time wait until the queue is not empty, then dequeue the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
       timeout_msec: timeout value in millisecond.
@return if ok: 0;
        if error: <0;
        timeout: 1;
*************************************************/
int xqueue_timewait_and_dequeue(hqueue_t hqueue, void* data, uint32_t* data_size, int timeout_msec)
{
    return timewait_and_mkdata(hqueue, data, data_size, timeout_msec, true);
}



/*************************************************
@brief wait until the queue is not empty, then dequeue the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_dequeue(hqueue_t hqueue, void* data, uint32_t* data_size)
{
    return xqueue_timewait_and_dequeue(hqueue, data, data_size, -1);
}


/*************************************************
@brief try read but not dequeue the head data from the queue,
       ok if queue is not empty, faile if the queue is empty.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: true;
        if fail: false;
*************************************************/
bool xqueue_try_front(hqueue_t hqueue, void* data, uint32_t* data_size)
{
    assert(hqueue != NULL);
    xqueue_t* queue = (xqueue_t*)hqueue;
    bool ret = true;
    pthread_mutex_lock(&(queue->mutex));
    if (queue->count == 0)
    {
        ret = false;
        goto end;
    }

    mkdata_for_user(queue, data, data_size);

end:
    pthread_mutex_unlock(&(queue->mutex));
    return ret;
}

/*************************************************
@brief time wait until the queue is not empty, then read but not dequeue the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
       timeout_msec: timeout value in millisecond.
@return if ok: 0;
        if error: <0;
        timeout: 1;
*************************************************/
int xqueue_timewait_and_front(hqueue_t hqueue, void* data, uint32_t* data_size, int timeout_msec)
{
    return timewait_and_mkdata(hqueue, data, data_size, timeout_msec, false);
}



/*************************************************
@brief wait until the queue is not empty, then read but not pop the data.
@param hqueue: the handle of the queue.
       data: the data buffer;
       data_size: the size of the data;
@return if ok: 0;
        if fail: <0;
*************************************************/
int xqueue_wait_and_front(hqueue_t hqueue, void* data, uint32_t* data_size)
{
    return xqueue_timewait_and_front(hqueue, data, data_size, -1);
}


/*************************************************
@brief get how many data in the queu
@param hqueue: the handle of the queue.
@return data cout
*************************************************/
uint32_t xqueue_size(hqueue_t hqueue)
{
    assert(hqueue != NULL);
    xqueue_t* queue = (xqueue_t*)hqueue;
    uint32_t size = 0;
    pthread_mutex_lock(&(queue->mutex));
    size = queue->count;
    pthread_mutex_unlock(&(queue->mutex));
    return size;
}


/*************************************************
@brief check the max length of the queue support
@param hqueue: the handle of the queue.
@return max length
*************************************************/
uint32_t xqueue_msg_max_len(hqueue_t hqueue)
{
    assert(hqueue != NULL);
    xqueue_t* queue = (xqueue_t*)hqueue;
    return queue->msg_max_len;
}

/*************************************************
@brief set the max item count of the queue.
       when the item number reachs the max count, the oldest item will be dropped.
@param hqueue: the handle of the queue.
       max_count: =0 means no limit; >0  means count is the max number.
@return void
*************************************************/
void xqueue_set_max_count(hqueue_t hqueue, uint32_t max_count)
{
    assert(hqueue != NULL);
    xqueue_t* queue = (xqueue_t*)hqueue;
    pthread_mutex_lock(&(queue->mutex));
    queue->max_count = max_count;
    pthread_mutex_unlock(&(queue->mutex));
}
