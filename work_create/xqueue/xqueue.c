#include "xqueue.h"
#include "xqueue_impl.h"

void* xqueue_new()
{
    return xqueue_impl_new();
}

int   xqueue_destory(void* hQueue)
{
    return xqueue_impl_destory((xqueue_impl_t*)hQueue);
}

int  xqueue_push(void* hQueue, const void* pData, uint32_t data_size)
{
    return xqueue_impl_push((xqueue_impl_t*)hQueue, pData, data_size);
}

bool  xqueue_empty(void* hQueue)
{
    return xqueue_impl_empty((xqueue_impl_t*)hQueue);
}

bool  xqueue_try_pop(void* hQueue, void** ppData, uint32_t* pData_size)
{
    return xqueue_impl_try_pop((xqueue_impl_t*)hQueue, ppData, pData_size);
}

int  xqueue_wait_and_pop(void* hQueue, void** ppData, uint32_t* pData_size)
{
    return xqueue_impl_wait_and_pop((xqueue_impl_t*)hQueue, ppData, pData_size);
}

bool  xqueue_try_front(void* hQueue, void** ppData, uint32_t* pData_size)
{
    return xqueue_impl_try_front((xqueue_impl_t*)hQueue, ppData, pData_size);
}

int  xqueue_wait_and_front(void* hQueue, void** ppData, uint32_t* pData_size)
{
    return xqueue_impl_wait_and_front((xqueue_impl_t*)hQueue, ppData, pData_size);
}

uint32_t xqueue_size(void* hQueue)
{
    return xqueue_impl_size((xqueue_impl_t*)hQueue);
}

void  xqueue_free(void* pData)
{
    return xqueue_impl_free(pData);
}

