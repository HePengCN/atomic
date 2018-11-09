#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "xqueue.h"
#include "xqueue_impl.h"

static int mkdata_for_user(xqueue_impl_t* hQueue, void* pData, uint32_t* pData_size)
{
    *pData_size = hQueue->head->data_size;
    memcpy(pData, hQueue->head->data, *pData_size);
    return 0;
}

xqueue_impl_t* xqueue_impl_new(uint32_t msgmaxlen)
{
    xqueue_impl_t* hQueue = (xqueue_impl_t*)malloc(sizeof(xqueue_impl_t));
    memset(hQueue, 0, sizeof(xqueue_impl_t));
    hQueue->msgmaxlen = msgmaxlen;
    pthread_cond_init(&(hQueue->cond), NULL);
    pthread_mutex_init(&(hQueue->mutex), NULL);
    return hQueue;
}

int xqueue_impl_destory(xqueue_impl_t* hQueue)
{
    assert(NULL != hQueue);

    if(0 != pthread_cond_destroy(&(hQueue->cond))) {
        return -1;
    }
    if(0 != pthread_mutex_destroy(&(hQueue->mutex))) {
        return -1;
    }

    xnode_t *tmp = NULL;
    while(!xqueue_empty(hQueue)) {
        tmp = hQueue->head->next;
        free(hQueue->head);
        hQueue->head = tmp;
        hQueue->count--;
    }
    free(hQueue);
    return 0;
}

int  xqueue_impl_push(xqueue_impl_t* hQueue, const void* pData, uint32_t data_size)
{
    assert(NULL != hQueue);
    assert(data_size <= hQueue->msgmaxlen);
    int ret = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    xnode_t* new = (xnode_t*)malloc(sizeof(xnode_t) + data_size);
    if(NULL == new) {
        ret = -1;
        goto end;
    }
    memset(new, 0, sizeof(xnode_t) + data_size);
    memcpy(new->data, pData, data_size);
    new->data_size = data_size;

    if(0 == hQueue->count) {
        hQueue->head = new;
    } else {
        hQueue->tail->next = new;
    }
    hQueue->tail = new;
    hQueue->count++;

end:
    if(0 == ret) {
        pthread_cond_signal(&(hQueue->cond));
    }
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;
}

bool  xqueue_impl_empty(xqueue_impl_t* hQueue)
{
    bool ret = true;
    assert(NULL != hQueue);
    pthread_mutex_lock(&(hQueue->mutex));
    ret = (0 == hQueue->count);
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;
}

bool  xqueue_impl_try_pop(xqueue_impl_t* hQueue, void* pData, uint32_t* pData_size)
{
    assert(NULL != hQueue);
    bool ret = true;
    pthread_mutex_lock(&(hQueue->mutex));
    if(0 == hQueue->count) {
        ret = false;
        goto end;
    }

    if (0 != mkdata_for_user(hQueue, pData, pData_size)) {
        ret = false;
        goto end;
    }

    xnode_t *tmp = hQueue->head;
    hQueue->head = hQueue->head->next;
    free(tmp);
    hQueue->count--;

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;
}

int  xqueue_impl_wait_and_pop(xqueue_impl_t* hQueue, void* pData, uint32_t* pData_size)
{
    assert(NULL != hQueue);
    int ret = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    if(0 == hQueue->count) {
        pthread_cond_wait(&(hQueue->cond), &(hQueue->mutex));
    }

    if (0 != mkdata_for_user(hQueue, pData, pData_size)) {
        ret = -1;
        goto end;
    }

    xnode_t *tmp = hQueue->head;
    hQueue->head = hQueue->head->next;
    free(tmp);
    hQueue->count--;

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;

}

bool  xqueue_impl_try_front(xqueue_impl_t* hQueue, void* pData, uint32_t* pData_size)
{
    assert(NULL != hQueue);
    bool ret = true;
    pthread_mutex_lock(&(hQueue->mutex));
    if(0 == hQueue->count) {
        ret = false;
        goto end;

    }

    if (0 != mkdata_for_user(hQueue, pData, pData_size)) {
        ret = false;
        goto end;
    }

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;

}

int  xqueue_impl_wait_and_front(xqueue_impl_t* hQueue, void* pData, uint32_t* pData_size)
{
    assert(NULL != hQueue);
    int ret = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    if(0 == hQueue->count) {
        pthread_cond_wait(&(hQueue->cond), &(hQueue->mutex));
    }

    if (0 != mkdata_for_user(hQueue, pData, pData_size)) {
        ret = -1;
        goto end;
    }

end:
    pthread_mutex_unlock(&(hQueue->mutex));
    return ret;

}

uint32_t xqueue_impl_size(xqueue_impl_t* hQueue)
{
    assert(NULL != hQueue);
    uint32_t size = 0;
    pthread_mutex_lock(&(hQueue->mutex));
    size = hQueue->count;
    pthread_mutex_unlock(&(hQueue->mutex));
    return size;
}

void  xqueue_impl_free(void* pData)
{
    if(NULL == pData) {
        return;
    }
    free(pData);
}

