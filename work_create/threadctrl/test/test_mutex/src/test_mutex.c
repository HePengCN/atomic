#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define gettid() syscall(__NR_gettid)


void dump(int signo)
{
    printf("%s, tid: %d\n", __FUNCTION__, (int)gettid());

    void *buffer[30] = {0};
    size_t size;
    char **strings = NULL;
    size_t i = 0;

    size = backtrace(buffer, 30);
    fprintf(stdout, "Obtained %zd stack frames.nm\n", size);
    strings = backtrace_symbols(buffer, size);
    if (strings == NULL)
    {
        perror("backtrace_symbols.");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < size; i++)
    {
        fprintf(stdout, "%s\n", strings[i]);
    }
    free(strings);
    strings = NULL;
    exit(0);
}


#if 0
void* start_routin2(void* arg)
{
    pthread_mutex_t* pmutex = arg;

    pthread_mutex_unlock(pmutex);

    printf("new thread2 unlock other, exit with lock __owner: %u\n", pmutex->__data.__owner);

    return arg;
}
#endif

static void unlock(pthread_mutex_t* pmutex)
{
    pthread_mutex_lock(pmutex);

    printf("new thread exit with lock __owner: %u\n", pmutex->__data.__owner);
}

void* start_routin(void* arg)
{
    fprintf(stderr, "%s, tid: %d\n", __FUNCTION__, (int)gettid());

#if 0
    unlock(arg);
#else
    pthread_mutex_t* pmutex = arg;

    pthread_mutex_lock(pmutex);

    printf("new thread exit with lock __owner: %u\n", pmutex->__data.__owner);
#endif


    return arg;
}

void test_sigbus()
{
    printf("sigbus test start.\n");

    char buf[8];
    memset(buf, -1, sizeof(buf));

    char* cp = buf + 1;
    int* ip = (int*)cp;

    printf("cp:%p, val: 0x%.2x\n", cp, *cp);
    printf("ip:%p, val: 0x%.8x\n", ip, *ip);

    *ip = 0x22;

    printf("ip:%p, val: 0x%.8x\n", ip, *ip);

    struct data
    {
        char a;
        short b __attribute__((packed));
        int c __attribute__((packed));
    };

    struct data s;
    s.a = 0x00;
    s.b = 0x00ff;
    s.c = 0xff00ff00;
    printf("sizeof(s): %d\n", sizeof(s));
    printf("&s.a: %p, &s.b: %p, &s.c: %p\n", &s.a, &s.b, &s.c);
    int* p = (int*)(&s.b);
    printf("p: %p, *p: 0x%.8x\n", p, *p);

    printf("sigbus test passed.\n");
}

int a __attribute__((section(".data"))) = 1;

void __attribute__((weak)) func(void);

int main(int argc, char* argv[])
{

    printf("func: %p\n", func);

    test_sigbus();

    if (signal(SIGSEGV, dump) == SIG_ERR)
    {
        perror("can't catch SIGSEGV");
    }

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
    pthread_create(&thd, NULL, start_routin, NULL/*&mutex*/);
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

    ret = pthread_mutex_destroy(&mutex);
    printf("main thread destroy: ret: %d, __owner: %u\n", ret, mutex.__data.__owner);

    return 0;
}
