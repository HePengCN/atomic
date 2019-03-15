#include <pthread.h>
#include <stdio.h>


void* start_routin2(void* arg)
{
    pthread_mutex_t* pmutex = arg;

    pthread_mutex_unlock(pmutex);

    printf("new thread2 unlock other, exit with lock __owner: %u\n", pmutex->__data.__owner);

    return arg;
}


void* start_routin(void* arg)
{
    pthread_mutex_t* pmutex = arg;

    pthread_mutex_lock(pmutex);

    printf("new thread exit with lock __owner: %u\n", pmutex->__data.__owner);

    return arg;
}

int main(int argc, char* argv[])
{
    int ret = 0;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    printf("init: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    ret = pthread_mutex_lock(&mutex);

    printf("main thread locked: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    ret = pthread_mutex_unlock(&mutex);

    printf("main thread unlocked: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    ret = pthread_mutex_unlock(&mutex);

    printf("main thread unlocked again: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);


    pthread_mutex_lock(&mutex);

    pthread_t thd;
    pthread_create(&thd, NULL, start_routin, &mutex);
    pthread_mutex_unlock(&mutex);

    pthread_join(thd, NULL);

#if 0
    pthread_create(&thd, NULL, start_routin2, &mutex);

    pthread_join(thd, NULL);
#endif

    ret = pthread_mutex_trylock(&mutex);
    printf("main thread trylocked after son thread: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    ret = pthread_mutex_unlock(&mutex);

    printf("main thread unlocked son thread: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    ret = pthread_mutex_trylock(&mutex);
    printf("main thread trylocked after son thread again: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    ret = pthread_mutex_unlock(&mutex);

    printf("main thread unlocked itself: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    ret = pthread_mutex_destroy(&(hthd->mutex));
    printf("main thread destroy: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    return 0;
}
